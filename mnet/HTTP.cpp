#include "pch.h"

#include "CurlUtils.h"

#include "HTTP.h"

MLOG_DECLARE_LOGGER(http);

namespace mnet {

namespace {

class AsyncTask : public mstd::reference_counter<AsyncTask>, public boost::noncopyable {
public:
    virtual void start(CURLM * curlm) = 0;
    virtual void done(int code) = 0;

    AsyncTask()
        : curl_(0) {}

    virtual ~AsyncTask()
    {
        if(curl_)
        {
            curl_multi_remove_handle(curlm_, curl_);
            curl_easy_cleanup(curl_);
        }
    }

    static void uiEnqueue(const boost::function<void()> & action);
protected:
    CURL * curl_;

    void initCurl(CURLM * curlm, const std::string & url, const std::string & cookies = std::string())
    {
        curlm_ = curlm;
        curl_ = createCurl(url, cookies);
        curl_easy_setopt(curl_, CURLOPT_PRIVATE, this);
        curl_multi_add_handle(curlm_, curl_);
    }
private:
    CURLM * curlm_;
};

typedef boost::intrusive_ptr<AsyncTask> AsyncTaskPtr;

class AsyncHTTP {
public:
    void addTask(const AsyncTaskPtr & task)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        queue_.push_back(task);
    }

    ~AsyncHTTP()
    {
        thread_.interrupt();
        thread_.join();
    }

    AsyncHTTP()
    {
        thread_ = boost::thread(&AsyncHTTP::execute, this);
    }
private:
    void execute()
    {
        try {
            std::vector<AsyncTaskPtr> tasks;
            CurlMultiHandle multi(curl_multi_init());
            fd_set readfs, writefs, excfs;
            int maxfd;

            while(!boost::this_thread::interruption_requested())
            {
                size_t oldTasks = tasks.size();
                {
                    boost::lock_guard<boost::mutex> lock(mutex_);
                    tasks.insert(tasks.end(), queue_.begin(), queue_.end());
                    queue_.clear();
                }
                for(size_t i = oldTasks, size = tasks.size(); i != size; ++i)
                    tasks[i]->start(*multi);
                int rh = 0;
                while(curl_multi_perform(*multi, &rh) == CURLM_CALL_MULTI_PERFORM);
                CURLMsg * msg;
                while((msg = curl_multi_info_read(*multi, &rh)) != 0)
                {
                    if(msg->msg == CURLMSG_DONE)
                    {
                        AsyncTask * task;
                        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &task);
                        long code;
                        curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code);
                        if(code >= 300)
                            MLOG_MESSAGE(Warning, "done with code: " << code << ", result: " << msg->data.result);
                        else
                            MLOG_MESSAGE(Info, "done with code: " << code << ", result: " << msg->data.result);

                        bool failed = msg->data.result != CURLE_WRITE_ERROR && (msg->data.result != CURLE_OK || code != 200);
                        AsyncTaskPtr tp(task);
                        for(std::vector<AsyncTaskPtr>::iterator i = tasks.begin(), end = tasks.end(); i != end; ++i)
                            if(i->get() == task)
                            {
                                tasks.erase(i);
                                break;
                            }
                        task->done(failed ? (code ? code : -1) : 0);
                    }
                }

                FD_ZERO(&readfs);
                FD_ZERO(&writefs);
                FD_ZERO(&excfs);
                maxfd = 0;
                CURLMcode cr = curl_multi_fdset(*multi, &readfs, &writefs, &excfs, &maxfd);
                if(cr != CURLM_OK)
                    MLOG_ERROR("curl_multi_fdset failed: " << cr);
                if(maxfd != -1)
                {
                    timeval timeout = { 0, 10000 };
                    int res = select(maxfd + 1, &readfs, &writefs, &excfs, &timeout);
                    if(res == -1)
                    {
                        MLOG_MESSAGE(Error, "select failed: " << res);
                        break;
                    }
                } else {
                    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
                }
            }
        } catch(boost::thread_interrupted&) {
        }
    }

    boost::thread thread_;
    boost::mutex mutex_;
    std::vector<AsyncTaskPtr> queue_;
};

class GetDataAsync : public AsyncTask {
public:
    GetDataAsync(const std::string & url, const AsyncDataHandler & handler, const std::string & cookies)
        : url_(url), handler_(handler), cookies_(cookies) {}

    GetDataAsync(const std::string & url, const AsyncDataExHandler & handler, const std::string & cookies)
        : url_(url), handlerEx_(handler), cookies_(cookies) {}

    void start(CURLM * curlm)
    {
        initCurl(curlm, url_, cookies_);
        void * self = this;
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &GetDataAsync::write);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, self);
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 1);
        if(!handlerEx_.empty())
        {
            curl_easy_setopt(curl_, CURLOPT_WRITEHEADER, &header_);
            curl_easy_setopt(curl_, CURLOPT_HEADER, 0);
            curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, &GetDataAsync::writeHeader);
        }
    }
    
    void done(int ec)
    {
        MLOG_MESSAGE_EX(ec ? mlog::llWarning : mlog::llDebug, "get data: " << url_ << ", code: " << ec);
        if(handlerEx_.empty())
            uiEnqueue(boost::bind(handler_, ec, data_));
        else
            uiEnqueue(boost::bind(handlerEx_, ec, data_, header_));
    }
private:
    static size_t write(const char* buf, size_t size, size_t nmemb, GetDataAsync * self)
    {
        std::string & data = self->data_;
        data.insert(data.end(), buf, buf + size * nmemb);
        return size * nmemb;
    }
    
    static size_t writeHeader(const char * buf, size_t size, size_t nmemb, std::string * header)
    {
        MLOG_MESSAGE(Debug, "writeHeader: " << std::string(buf, buf + size * nmemb));
        return appendToString(buf, size, nmemb, header);
    }
    
    std::string url_;
    AsyncDataHandler handler_;
    AsyncDataExHandler handlerEx_;
    std::string cookies_;
    std::string data_;
    std::string header_;
};

class GetFileAsync : public AsyncTask {
public:
    GetFileAsync(const std::string & url, const boost::filesystem::wpath & fname, const AsyncHandler & handler, const ProgressHandler & prog)
        : progress_(0), url_(url), fname_(fname), out_(0), handler_(handler), prog_(prog)
    {
    }

    ~GetFileAsync()
    {
        if(out_)
            fclose(out_);
    }

    void start(CURLM * curlm)
    {
        initCurl(curlm, url_);
        void * self = this;
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &GetFileAsync::write);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, self);
        curl_easy_setopt(curl_, CURLOPT_PROGRESSFUNCTION, &GetFileAsync::progress);
        curl_easy_setopt(curl_, CURLOPT_PROGRESSDATA, self);
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, prog_.empty() ? 1 : 0);

        out_ = mstd::wfopen(fname_, "wb");
    }

    void done(int ec)
    {
        if(ec)
            MLOG_MESSAGE(Warning, "failed to download: " << url_ << ", code: " << ec);
        if(out_)
        {
            fclose(out_);
            out_ = 0;
        }
        uiEnqueue(boost::bind(handler_, ec));
    }

/*    void operator()()
    {
        try {
            MLOG_MESSAGE(Debug, "async file: " << url_ << " => " << mstd::utf8(fname_.external_file_string()));
            http_->download(url_, fname_, boost::bind(&GetFileAsync::progress, this, _1));
            uiEnqueue(boost::bind(handler_, boost::system::error_code()));
        } catch(http_t::exception &) {
            uiEnqueue(boost::bind(handler_, boost::system::errc::make_error_code(boost::system::errc::protocol_error)));
        }
    }*/

private:
    static int progress(GetFileAsync * self, double t, double d, double ultotal, double ulnow)
    {
        size_t pr = static_cast<size_t>(t ? (100.0 * d / t) : 0);
        if(self->progress_ != pr)
        {
            self->progress_ = pr;
            uiEnqueue(boost::bind(self->prog_, pr));
        }
        return 0;
    }

    static size_t write(const char* buf, size_t size, size_t nmemb, GetFileAsync * self)
    {
        FILE * out = self->out_;
        if(out)
        {
            size_t written = fwrite(buf, size, nmemb, self->out_);
            if(written != size * nmemb)
            {
                MLOG_ERROR("write failed: " << written << " vs " << size * nmemb);
            }
        }
        return size * nmemb;
    }

    size_t progress_;
    std::string url_;
    boost::filesystem::wpath fname_;
    FILE * out_;
    AsyncHandler handler_;
    ProgressHandler prog_;
};

class HTTP {
    MSTD_SINGLETON_INLINE_DEFINITION(HTTP);
public:
    const std::string & userAgent()
    {
        return userAgent_;
    }

    void setUserAgent(const std::string & value)
    {
        userAgent_ = value;
    }

    void setProxy(const Proxy & proxy)
    {
        boost::unique_lock<boost::shared_mutex> lock(proxyMutex_);
        proxy_ = proxy;
	    read_ie_proxy_settings();
    }

    void uiEnqueue(const Action & action)
    {
        uiEnqueuer_(action);
    }

    void setUIEnqueuer(const UIEnqueuer & enqueuer)
    {
        uiEnqueuer_ = enqueuer;
    }

    AsyncHTTP & asyncHTTP()
    {
        return asyncHTTP_;
    }

    CURL * createCurl(const std::string & url, const std::string & cookies)
    {
	    CURL* curl = curl_easy_init();
	    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    if(!cookies.empty())
		    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());

	    /* configure curl instance */
	
        {
            boost::shared_lock<boost::shared_mutex> lock(proxyMutex_);

	        bool use_proxy = proxy_.active || (proxy_.useSystem && ieUsesProxy_);

	        if(use_proxy)
            {
		        std::string proxy;
		        long port;
		        if(proxy_.useSystem)
                {
			        proxy = ieProxy_.host;
			        port = ieProxy_.port;
		        } else {
			        proxy = proxy_.host; 	
			        port = proxy_.port;
		        }
		        CURLcode cr;
		        cr = curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
		        cr = curl_easy_setopt(curl, CURLOPT_PROXYPORT, port);
		        /* 
		            Maybe more proxy tuning requered?	
		            cr = curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
		            cr = curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY); 
		        */
		        cr = curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, proxy_.login.c_str());
		        cr = curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, proxy_.password.c_str());
	        }
        }

	    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);

	    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	    curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");
	    curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 1);
	
	    #ifdef __APPLE__
	    /* setting this option is needed to allow libcurl to work
	        multithreaded with standard DNS resolver. see libcurl
	        documentation about the side-effects */
	    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	    #endif
	
        curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent().c_str());

        return curl;
    }

    int getRemoteFileSizeEx(std::string& uri, filesize_t & out)
    {
        try {
            GetEx data = { uri, std::string(), 0 };
            getEx(data);
            if(!error(data.headerData))
            {
                out = data.headerData.contentLength;
                uri.swap(data.url);
                return 0;
            } else
                return data.headerData.statusCode;
        } catch(exception &){
            return 444;
        }
    }
private:
    HTTP()
        : ieUsesProxy_(false)
    {
        proxy_.active = false;
        ieProxy_.active = false;
    }

    void read_ie_proxy_settings()
    {
	    /* 
	      Maybe, IE automatic proxy configuration also must be processed:
	      http://stackoverflow.com/questions/202547/how-do-i-find-out-the-browsers 
	      (WinHttpGetProxyForUrl) 
	    */


	    ieUsesProxy_ = false;
	
	    #ifdef MSC_VER_
	    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG pc;
	    BOOL r = WinHttpGetIEProxyConfigForCurrentUser(&pc);
	    if(r == FALSE || pc.lpszProxy == NULL) return;
	    string ps = CW2A(pc.lpszProxy);
	    int p = ps.rfind(":");
	    if(p == ps.npos) return;
	    try { 
		    ie_proxy_port = lexical_cast<int>(ps.substr(p + 1));
		    ie_proxy = ps.substr(0, p);
            eiUsesProxy_ = true;
	    } catch(bad_cast&) { return; }
	    #endif
    }

    struct GetEx;

    void getEx(GetEx & data)
    {
        MLOG_MESSAGE(Debug, "http request: " << data.url);

        CURL* curl = createCurl(data.url, data.cookies);
        if(!data.body)
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
        else {
            data.body->clear();
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendToString);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, data.body);
        }
        CURLcode cr = curl_easy_perform(curl);

        MLOG_MESSAGE(Debug, "http result: " << cr);
        BOOST_SCOPE_EXIT((curl)) {
            curl_easy_cleanup(curl); 
        } BOOST_SCOPE_EXIT_END;
        if(cr != CURLE_OK)
        {
            MLOG_MESSAGE(Warning, "perform error: " << cr);
            char buf[0x20];
            throw exception() << mstd::error_message(std::string("curl perform error: ") + mstd::itoa(static_cast<int>(cr), buf));
        }

        {
            char * str = 0;
            CURLcode res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &str);
            data.headerData.contentType = res == CURLE_OK ? str : "<unknown>";
            long code;
            res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
            data.headerData.statusCode = res == CURLE_OK ? code : 444;
            double contentLength;
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
            data.headerData.contentLength = static_cast<filesize_t>(contentLength);
        }
        if(data.body)
        {
            const std::string & body = *data.body;
            MLOG_MESSAGE(Debug, "http response, size: " << body.size() << ", type: " << data.headerData.contentType); 
            size_t len = std::min<size_t>(body.size(), 0x400);
            bool ok = true;
            for(size_t i = 0; i != len; ++i)
                if((body[i] < 32 || body[i] & 0x80) && body[i] != '\r' && body[i] != '\n')
                {
                    ok = false;
                    break;
                }
                if(ok)
                    MLOG_MESSAGE(Debug, "http response data: " << body.substr(0, len));
                else
                    MLOG_MESSAGE(Debug, "http response data: " << mlog::dump(body.c_str(), len));
        }

        
        if(data.headerData.statusCode == 301 || data.headerData.statusCode == 302)
        {
            MLOG_MESSAGE(Info, "http redirection...");

            char * url;
            CURLcode res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &url);
            if(res == CURLE_OK)
            {
                data.url = url;
                getEx(data);
            }
        }
    }

    struct HeaderData {
        int statusCode;
        filesize_t contentLength;
        std::string contentType;
    };

    static bool error(const HeaderData & hd)
    {
        return hd.statusCode >= 400;
    }

    struct GetEx {
        std::string url;
        std::string cookies;
        std::string * body;
        HeaderData headerData;
    };

    typedef mstd::own_exception<HTTP> exception;

    boost::shared_mutex proxyMutex_;
    bool ieUsesProxy_;
    Proxy proxy_;
    Proxy ieProxy_;
    std::string userAgent_;
    UIEnqueuer uiEnqueuer_;
    AsyncHTTP asyncHTTP_;
};

void AsyncTask::uiEnqueue(const boost::function<void()> & action)
{
    HTTP::instance().uiEnqueue(action);
}

}

// free functions

const std::string & userAgent()
{
    return HTTP::instance().userAgent();
}

void setUserAgent(const std::string & value)
{
    HTTP::instance().setUserAgent(value);
}

void setProxy(const Proxy & proxy)
{
    HTTP::instance().setProxy(proxy);
}

void setUIEnqueuer(const UIEnqueuer & enqueuer)
{
    HTTP::instance().setUIEnqueuer(enqueuer);
}

void getDataAsync(const std::string & url, const AsyncDataHandler & handler, const std::string & cookies)
{
    MLOG_DEBUG("getDataAsync(" << url << ", " << cookies << ')');

    HTTP::instance().asyncHTTP().addTask(new GetDataAsync(url, handler, cookies));
}

boost::property_tree::ptree parseXml(const std::string& data)
{
    std::istringstream inp(data);
    boost::property_tree::ptree result;
    try {
        boost::property_tree::read_xml(inp, result);
    } catch(boost::property_tree::xml_parser_error&) {
    }
    return result;
}

boost::property_tree::ptree parseJSON(const std::string& data)
{
    std::istringstream inp(data);
    boost::property_tree::ptree result;
    try {
        boost::property_tree::read_json(inp, result);
    } catch(boost::property_tree::json_parser_error&) {
    }
    return result;
}

namespace {

struct XmlParser {
    static inline boost::property_tree::ptree parse(const std::string & data)
    {
        return parseXml(data);
    }
};

struct JSONParser {
    static inline boost::property_tree::ptree parse(const std::string & data)
    {
        return parseJSON(data);
    }
};

template<class Parser>
class GetTreeAsyncHandler {
public:
    explicit GetTreeAsyncHandler(const AsyncXmlHandler & handler)
        : handler_(handler) {}
    
    void operator()(int ec, const std::string & data) const
    {
        if(!ec)
        {
            MLOG_DEBUG("received tree: " << data);
            handler_(ec, Parser::parse(data));
        } else
            handler_(ec, boost::property_tree::ptree());
    }
private:
    AsyncXmlHandler handler_;
};

}

void getXmlAsync(const std::string & url, const AsyncXmlHandler & handler, const std::string & cookies)
{
    getDataAsync(url, GetTreeAsyncHandler<XmlParser>(handler), cookies);
}

void getJSONAsync(const std::string & url, const AsyncXmlHandler & handler, const std::string & cookies)
{
    getDataAsync(url, GetTreeAsyncHandler<JSONParser>(handler), cookies);
}

void getFileAsync(const std::string & url, const boost::filesystem::wpath & path, const AsyncHandler & handler)
{
    MLOG_DEBUG("getFileAsync(" << url << ", " << mstd::utf8fname(path) << ')');

    HTTP::instance().asyncHTTP().addTask(new GetFileAsync(url, path, handler, boost::function<void(size_t)>()));
}

void getFileAsync(const std::string & url, const boost::filesystem::wpath & path, const AsyncHandler & handler, const ProgressHandler& progress)
{
    MLOG_DEBUG("getFileAsync(" << url << ", " << mstd::utf8fname(path) << ')');

    HTTP::instance().asyncHTTP().addTask(new GetFileAsync(url, path, handler, progress));
}

CURL * createCurl(const std::string & url, const std::string & cookies)
{
    return HTTP::instance().createCurl(url, cookies);
}

int getRemoteFileSizeEx(std::string& uri, filesize_t & out)
{
    return HTTP::instance().getRemoteFileSizeEx(uri, out);
}

}

#if 0

size_t curl_null_write_function(void *ptr, size_t size, size_t nmemb, void* stream){  
	return nmemb * size;  
}  

http_t::http_t()
{
	update_configuration();
    ::config::addConfigListener(boost::bind(&http_t::update_configuration, this));
}

boost::property_tree::ptree http_t::getXml(const std::string & url, const std::string & cookies)
{
    return parseXml(get(url, cookies));
}

void dummyProgress(size_t)
{
}

void http_t::getDataExAsync(const std::string & url, const AsyncDataExHandler & handler, const std::string & cookies)
{
    AsyncHTTP::instance().addTask(new GetDataAsync(url, handler, cookies));
}


std::string http_t::get(const std::string& url, const std::string& cookies)
{
    std::string result;
	GetEx data = { url, cookies, &result };
	getEx(data);
    if(http_error(data.headerData))
    {
        char buf[0x20];
        throw http_t::exception() << mstd::error_message(std::string("http error: ") + mstd::itoa(data.headerData.status_code, buf));
    }
    return result;
}

http_header_data_t http_t::parse_header(const std::string& header)
{
    MLOG_MESSAGE(Debug, "parse_header(" << header << ')');
    
    http_header_data_t hd;

	hd.content_length = 0;
	hd.content_type = "";
	hd.status_code = 0;

	{
		std::stringstream ss(header);
		std::string status_str;
		getline(ss, status_str);

		match_results<string::const_iterator> mr;
		if(regex_search(header, mr, regex(".+?\\s([0-9]+?)\\s(.+?)$"))){
			string code_str = string(mr[1].first, mr[1].second);
			string descr_str = string(mr[2].first, mr[2].second);
			hd.status_code = lexical_cast<int>(code_str);
			hd.status_string = descr_str;
		}

	}

	{
		match_results<string::const_iterator> mr;
		if(regex_search(header, mr, regex("Content-Length:\\s([0-9].+?)$"))){
			string size_str = string(mr[1].first, mr[1].second);
			try {
				hd.content_length = lexical_cast<long long>(size_str);
			} catch(bad_cast&) {}
		}
	} 


	{
		match_results<string::const_iterator> mr;
		if(regex_search(header, mr, regex("Content-Type:\\s(.+?)$"))){
			string str = string(mr[1].first, mr[1].second);
			hd.content_type = str;
		}						
	} 

	/*
	   // for parser debugging 
	log() << "\nheader_data::code : " << hd.status_code << endl
		  << "header_data::descr : " << hd.status_string << endl
		  << "header_data::content_length : " << hd.content_length << endl
		  << "header_data::content_type : " << hd.content_type << endl;
    */

	return hd;

}

#endif
