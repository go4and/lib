/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#pragma warning(disable: 4996)

#include "CurlUtils.h"
#include "JSON.h"

#include "HTTP.h"

MLOG_DECLARE_LOGGER(http);

namespace mnet {

namespace {

class AsyncTask : public mstd::reference_counter<AsyncTask>, public boost::noncopyable {
public:
    virtual void start(CURLM * curlm) = 0;
    virtual void done(int code) = 0;

    explicit AsyncTask()
        : curl_(0), noUI_(false) {}

    void noUI(bool value)
    {
        noUI_ = value;
    }

    virtual ~AsyncTask()
    {
        if(curl_)
        {
            curl_multi_remove_handle(curlm_, curl_);
            curl_easy_cleanup(curl_);
        }
    }

    template<class Action>
    void notify(const Action & action);
protected:
    CURL * curl_;

    mstd::rc_buffer blankBuffer();

    void initCurl(CURLM * curlm, const std::string & url, const std::string & cookies = std::string())
    {
        curlm_ = curlm;
        curl_ = createCurl(url, cookies);
        curl_easy_setopt(curl_, CURLOPT_PRIVATE, this);
        curl_multi_add_handle(curlm_, curl_);
    }
private:
    CURLM * curlm_;
    bool noUI_;
};

typedef boost::intrusive_ptr<AsyncTask> AsyncTaskPtr;

class AsyncHTTP {
public:
    void addTask(const AsyncTaskPtr & task)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        queue_.push_back(task);
        condition_.notify_one();
    }

    void cancelAll()
    {
        cancelAll_ = true;
    }

    ~AsyncHTTP()
    {
        thread_.interrupt();
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            condition_.notify_one();
        }
        thread_.join();
    }

    AsyncHTTP()
        : cancelAll_(false)
    {
        thread_ = boost::thread(&AsyncHTTP::execute, this);
    }
private:
    void execute()
    {
        char * curlVersion = curl_version();
        MLOG_DEBUG("curl version: " << curlVersion);
        conditionWait_ = true;

        try {
            std::vector<AsyncTaskPtr> tasks;
            CurlMultiHandle multi(curl_multi_init());

            while(!boost::this_thread::interruption_requested())
            {
                size_t oldTasks = tasks.size();
                {
                    boost::unique_lock<boost::mutex> lock(mutex_);
                    if(conditionWait_ && queue_.empty())
                        condition_.timed_wait(lock, boost::posix_time::seconds(1));
                    tasks.insert(tasks.end(), queue_.begin(), queue_.end());
                    queue_.clear();
                }

                startNewTasks(multi, tasks, oldTasks);
                processMessages(multi, tasks);
                waitIO(multi, tasks);
            }
        } catch(boost::thread_interrupted&) {
        }
    }

    void startNewTasks(CurlMultiHandle & multi, std::vector<AsyncTaskPtr> & tasks, size_t oldTasks)
    {
        bool cancel = cancelAll_.cas(false, true);
        if(!cancel)
        {
            for(size_t i = oldTasks, size = tasks.size(); i != size; ++i)
                tasks[i]->start(*multi);
        } else {
            for(std::vector<AsyncTaskPtr>::iterator i = tasks.begin(), end = tasks.end(); i != end; ++i)
                (*i)->done(600 + CURLE_ABORTED_BY_CALLBACK);
            tasks.clear();
        }
    }

    void processMessages(CurlMultiHandle & multi, std::vector<AsyncTaskPtr> & tasks)
    {
        int rh = 0;
        // MLOG_DEBUG("performing curl");
        for(;;)
        {
            CURLMcode code = curl_multi_perform(*multi, &rh);
            // MLOG_DEBUG("perform code: " << code);
            if(code != CURLM_CALL_MULTI_PERFORM)
                break;
        }

        CURLMsg * msg;
        // MLOG_DEBUG("reading messages");
        while((msg = curl_multi_info_read(*multi, &rh)) != 0)
        {
            // MLOG_DEBUG("message: " << msg->msg);
            if(msg->msg == CURLMSG_DONE)
            {
                AsyncTask * task;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &task);
                long code;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code);
                MLOG_MESSAGE_EX(code >= 300 ? mlog::llWarning : mlog::llInfo, "done with code: " << code << ", result: " << msg->data.result);

                bool failed = msg->data.result != CURLE_OK || code >= 300;
                AsyncTaskPtr tp(task);
                for(std::vector<AsyncTaskPtr>::iterator i = tasks.begin(), end = tasks.end(); i != end; ++i)
                    if(i->get() == task)
                    {
                        tasks.erase(i);
                        break;
                    }
                task->done(failed ? static_cast<int>(code >= 300 ? code : 600 + msg->data.result) : 0);
            }
        }
    }

    void waitIO(CurlMultiHandle & multi, std::vector<AsyncTaskPtr> & tasks)
    {
        conditionWait_ = false;
        
        fd_set readfs, writefs, excfs;
        int maxfd;

        FD_ZERO(&readfs);
        FD_ZERO(&writefs);
        FD_ZERO(&excfs);
        maxfd = 0;
        // MLOG_DEBUG("init fdset");
        CURLMcode cr = curl_multi_fdset(*multi, &readfs, &writefs, &excfs, &maxfd);
        if(cr != CURLM_OK)
            MLOG_ERROR("curl_multi_fdset failed: " << cr);
        if(maxfd != -1)
        {
            timeval timeout = { 0, 10000 };
            // MLOG_DEBUG("select");
            int res = select(maxfd + 1, &readfs, &writefs, &excfs, &timeout);
            if(res == -1)
            {
                int err = errno;
                MLOG_MESSAGE(Error, "select failed: " << err);
            }
        } else {
            if(!tasks.empty())
                boost::this_thread::sleep(boost::posix_time::milliseconds(10));
            else
                conditionWait_ = true;
        }
    }
    
    boost::mutex mutex_;
    boost::condition_variable condition_;
    boost::thread thread_;
    std::vector<AsyncTaskPtr> queue_;
    mstd::atomic<bool> cancelAll_;
    bool conditionWait_;
};

class DownloadTask : public AsyncTask {
public:
    DownloadTask(const std::string & url, const std::string & cookies, const ProgressHandler & prog)
        : url_(url), cookies_(cookies), prog_(prog), progress_(0)
    {
    }

    void start(CURLM * curlm)
    {
        MLOG_DEBUG("DownloadTask::start(" << curlm << ")");
        MLOG_DEBUG("DownloadTask, url = " << url() << ", cookies = " << cookies_);

        initCurl(curlm, url_, cookies_);
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, !prog_ ? 1 : 0);
        if(prog_)
        {
            void * self = this;
            curl_easy_setopt(curl_, CURLOPT_PROGRESSFUNCTION, &DownloadTask::progress);
            curl_easy_setopt(curl_, CURLOPT_PROGRESSDATA, self);
        }
        doStart();
    }

    virtual void doStart() = 0;

    inline const std::string & url() const { return url_; }
private:
    static int progress(DownloadTask * self, double t, double d, double ultotal, double ulnow)
    {
        size_t pr = static_cast<size_t>(t ? (100.0 * d / t) : 0);
        if(self->progress_ != pr)
        {
            self->progress_ = pr;
            self->notify(std::bind(self->prog_, static_cast<int>(pr)));
        }
        return 0;
    }

    std::string url_;
    std::string cookies_;
    ProgressHandler prog_;
    size_t progress_;
};

class GetDataAsync : public DownloadTask {
public:
    GetDataAsync(const Request & request)
        : DownloadTask(request.url(), request.cookies(), request.progressHandler()),
          handler_(request.handler()), directWriter_(request.directWriter()),
          rangeBegin_(request.rangeBegin()), rangeEnd_(request.rangeEnd()),
          postData_(request.postData()), 
          clientCertificate_(request.clientCertificate()),
          clientKey_(request.clientKey()),
          certificateAuthority_(request.certificateAuthority()),
          headers_(0)
    {
        noUI(request.noUI());
        const std::vector<std::string> & headers = request.headers();
        for(std::vector<std::string>::const_iterator i = headers.begin(), end = headers.end(); i != end; ++i)
        {
            curl_slist * item = new curl_slist;
            item->data = strdup(i->c_str());
            item->next = headers_;            
            headers_ = item;
        }
    }
    
    ~GetDataAsync()
    {
        while(headers_)
        {
            curl_slist * item = headers_;
            headers_ = item->next;

            free(item->data);
            delete item;
        }
    }

    void doStart()
    {
        MLOG_DEBUG("GetDataAsync::doStart(" << url() << ')');
        void * self = this;
        if(boost::get<AsyncSizeHandler>(&handler_))
            curl_easy_setopt(curl_, CURLOPT_NOBODY, 1);
        else if(boost::get<AsyncHandler>(&handler_))
        {
            curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &GetDataAsync::writeDirect);
            curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &directWriter_);
        } else {
            curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &GetDataAsync::write);
            curl_easy_setopt(curl_, CURLOPT_WRITEDATA, self);
            if(boost::get<AsyncDataExHandler>(&handler_))
            {
                curl_easy_setopt(curl_, CURLOPT_WRITEHEADER, self);
                curl_easy_setopt(curl_, CURLOPT_HEADER, 0);
                curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, &GetDataAsync::writeHeader);
            }
        }
        if(postData_)
        {
            curl_easy_setopt(curl_, CURLOPT_POST, 1L);
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, postData_.data());
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, static_cast<long>(postData_.size()));
        }
        if(rangeBegin_ >= 0)
        {
            char buf[0x40];
            char * p = buf;
            mstd::itoa(rangeBegin_, p);
            p += strlen(p);
            *p++ = '-';
            mstd::itoa(rangeEnd_ - 1, p);
            curl_easy_setopt(curl_, CURLOPT_RANGE, buf);
        }
        if(headers_)
            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
        if(certificateAuthority_ || clientCertificate_ || clientKey_)
        {
            curl_easy_setopt(curl_, CURLOPT_SSL_CTX_FUNCTION, &GetDataAsync::setupSSL);
            curl_easy_setopt(curl_, CURLOPT_SSL_CTX_DATA, this);
        }
        MLOG_DEBUG("GetDataAsync::doStart, done");
    }
    
    void done(int ec)
    {
        MLOG_MESSAGE_EX(ec ? mlog::llWarning : mlog::llDebug, "get data: " << url() << ", code: " << ec);

        if(const AsyncHandler * handler = boost::get<AsyncHandler>(&handler_))
            notify(std::bind(*handler, ec));
        else {
            if(!data_)
                data_ = blankBuffer();
            if(const AsyncDataHandler * handler = boost::get<AsyncDataHandler>(&handler_))
                notify(std::bind(*handler, ec, data_));
            else if(const AsyncDataExHandler * handler = boost::get<AsyncDataExHandler>(&handler_))
            {
                if(!header_)
                    header_ = blankBuffer();
                notify(std::bind(*handler, ec, data_, header_));
            } else if(const AsyncSizeHandler * handler = boost::get<AsyncSizeHandler>(&handler_))
            {
                filesize_t size;
                if(!ec)
                {
                    double contentLength;
                    CURLcode res = curl_easy_getinfo(curl_, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
                    size = res == CURLE_OK ? static_cast<filesize_t>(contentLength) : -1;
                } else
                    size = -1;
                notify(std::bind(*handler, ec, size));
            }
        }
    }
private:
    static CURLcode setupSSL(CURL * curl, void * sslctx, void * param)
    {
        GetDataAsync * self = static_cast<GetDataAsync*>(param);
        SSL_CTX * ctx = static_cast<SSL_CTX*>(sslctx);
        X509_STORE * store = SSL_CTX_get_cert_store(ctx);
        if(self->certificateAuthority_)
        {
            if(!X509_STORE_add_cert(store, static_cast<X509*>(self->certificateAuthority_)))
            {
                MLOG_ERROR("Failed to add CA");
                return CURLE_SSL_CACERT_BADFILE;
            }
        }

        if(self->clientCertificate_)
        {
            if(SSL_CTX_use_certificate(ctx, static_cast<X509*>(self->clientCertificate_)) != 1)
            {
                MLOG_ERROR("Failed to use certificate");
                return CURLE_SSL_CERTPROBLEM;
            }
        }

        if(self->clientKey_)
        {
            if(SSL_CTX_use_RSAPrivateKey(ctx, static_cast<RSA*>(self->clientKey_)) != 1)
            {
                MLOG_ERROR("Failed to use client key");
                return CURLE_SSL_CERTPROBLEM;
            }
        }

        return CURLE_OK ;        
    }

    static size_t write(const char* buf, size_t size, size_t nmemb, GetDataAsync * self)
    {
        MLOG_MESSAGE(Debug, "GetDataAsync::write(" << static_cast<const void*>(buf) << ", " << size << ", " << nmemb << ", " << self << ")");
        
        size *= nmemb;
        double contentLength;
        if(!self->data_)
        {
            CURLcode res = curl_easy_getinfo(self->curl_, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
            if(res == CURLE_OK && contentLength >= 0.0)
            {
                size_t length = static_cast<size_t>(contentLength);
                self->data_ = mstd::rc_buffer(length);
            } else {
                self->data_ = mstd::rc_buffer(size);
            }
            self->data_.resize(0);
        }
        self->data_.append(buf, size);
        return size;
    }
    
    static size_t writeHeader(const char * buf, size_t size, size_t nmemb, GetDataAsync * self)
    {
        MLOG_MESSAGE(Debug, "GetDataAsync::writeHeader(" << static_cast<const void*>(buf) << ", " << size << ", " << nmemb << ", " << self << ")");

        size *= nmemb;
        if(!self->header_)
        {
            self->header_ = mstd::rc_buffer(size);
            self->header_.resize(0);
        }
        self->header_.append(buf, size);
        return size;
    }

    static size_t writeDirect(const char * buf, size_t size, size_t nmemb, DirectWriter * writer)
    {
        return (*writer)(buf, size * nmemb);
    }
    
    AsyncRequestHandler handler_;
    DirectWriter directWriter_;
    mstd::rc_buffer data_;
    mstd::rc_buffer header_;
    filesize_t rangeBegin_, rangeEnd_;
    mstd::rc_buffer postData_;
    curl_slist * headers_;
    void * clientCertificate_;
    void * clientKey_;
    void * certificateAuthority_;
};

class GetFileAsync : public DownloadTask {
public:
    GetFileAsync(const std::string & url, const boost::filesystem::wpath & fname, const AsyncHandler & handler, const ProgressHandler & prog)
        : DownloadTask(url, std::string(), prog), fname_(fname), out_(0), handler_(handler)
    {
    }

    ~GetFileAsync()
    {
        if(out_)
            fclose(out_);
    }

    void doStart()
    {
        void * self = this;
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &GetFileAsync::write);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, self);
    }

    void done(int ec)
    {
        if(ec)
            MLOG_MESSAGE(Warning, "failed to download: " << url() << ", code: " << ec);
        if(out_)
        {
            fclose(out_);
            out_ = 0;
            if(ec)
            {
                boost::system::error_code err;
                remove(fname_, err);
            }
        }
        notify(std::bind(handler_, ec));
    }
private:
    static size_t write(const char* buf, size_t size, size_t nmemb, GetFileAsync * self)
    {
        MLOG_DEBUG("GetFileAsync::write(" << size << ", " << nmemb << ", " << self << ')');

        FILE * out = self->out_;
        if(!out)
        {
            out = self->out_ = mstd::wfopen(self->fname_, "wb");
            if(!out)
            {
                int err = errno;
                MLOG_ERROR("failed to open " << mstd::utf8fname(self->fname_) << ", err: " << err << ", for: " << self->url());
                return 0;
            }
        }
        size_t written = fwrite(buf, 1, size * nmemb, self->out_);
        if(written != size * nmemb)
        {
            MLOG_ERROR("write failed: " << written << " vs " << size * nmemb);
        }
        return written;
    }

    boost::filesystem::wpath fname_;
    FILE * out_;
    AsyncHandler handler_;
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

	    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);

	    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_version_info_data * versionData = curl_version_info(CURLVERSION_NOW);
        if(versionData->features & CURL_VERSION_LIBZ)
        {
            curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");
            curl_easy_setopt(curl, CURLOPT_HTTP_CONTENT_DECODING, 1);
        }
	
	    #if !BOOST_WINDOWS
	    /* setting this option is needed to allow libcurl to work
	        multithreaded with standard DNS resolver. see libcurl
	        documentation about the side-effects */
	    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	    #endif
	
        curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent().c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

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

    mstd::rc_buffer blankBuffer()
    {
        return blankBuffer_;
    }
private:
    HTTP()
        : ieUsesProxy_(false), blankBuffer_(0)
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
            data.headerData.statusCode = res == CURLE_OK ? static_cast<int>(code) : 444;
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
    mstd::rc_buffer blankBuffer_;
};

template<class Action>
void AsyncTask::notify(const Action & action)
{
    if(noUI_)
        const_cast<Action&>(action)(); // fix for std::bind in MSVC
    else
        HTTP::instance().uiEnqueue(action);
}

mstd::rc_buffer AsyncTask::blankBuffer()
{
    return HTTP::instance().blankBuffer();
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

void cancelAll()
{
    MLOG_DEBUG("cancelAll()");

    HTTP::instance().asyncHTTP().cancelAll();
}

void parseXml(boost::property_tree::ptree & result, const mstd::rc_buffer & data)
{
    std::istringstream inp(std::string(data.data(), data.size()));
    try {
        boost::property_tree::read_xml(inp, result);
    } catch(boost::property_tree::xml_parser_error&) {
    }
}

void parseJSON(boost::property_tree::ptree & result, const mstd::rc_buffer & data)
{
    ParseError err;
    parseJSON(data.data(), data.size(), result, err);
    if(err)
        MLOG_WARNING("invalid json: " << err.message());
}

namespace {

struct XmlParser {
    static inline void parse(boost::property_tree::ptree & tree, const mstd::rc_buffer & data)
    {
        parseXml(tree, data);
    }
};

struct JSONParser {
    static inline void parse(boost::property_tree::ptree & tree, const mstd::rc_buffer & data)
    {
        parseJSON(tree, data);
    }
};

template<class Parser>
class GetTreeAsyncHandler {
public:
    explicit GetTreeAsyncHandler(const AsyncPTreeHandler & handler)
        : handler_(handler) {}

    void operator()(int ec, const mstd::rc_buffer & data) const
    {
        if(!ec)
        {
            MLOG_DEBUG("received tree: " << mlog::dump(data.data(), data.size()));
            boost::property_tree::ptree tree;
            Parser::parse(tree, data);
            handler_(ec, tree);
        } else
            handler_(ec, boost::property_tree::ptree());
    }
private:
    AsyncPTreeHandler handler_;
};

}

Request & Request::xmlHandler(const AsyncPTreeHandler & value)
{
    return dataHandler(GetTreeAsyncHandler<XmlParser>(value));
}

Request & Request::jsonHandler(const AsyncPTreeHandler & value)
{
    return dataHandler(GetTreeAsyncHandler<JSONParser>(value));
}

void getFileAsync(const std::string & url, const boost::filesystem::wpath & path, const AsyncHandler & handler)
{
    MLOG_DEBUG("getFileAsync(" << url << ", " << mstd::utf8fname(path) << ')');

    HTTP::instance().asyncHTTP().addTask(new GetFileAsync(url, path, handler, std::function<void(size_t)>()));
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

void Request::run()
{
    MLOG_DEBUG("Request::run(" << *this << ')');

    HTTP::instance().asyncHTTP().addTask(new GetDataAsync(*this));
}

std::ostream & operator<<(std::ostream & out, const Request & request)
{
    out << "[url=" << request.url();
    if(request.rangeBegin() >= 0)
        out << ", range=" << request.rangeBegin() << "-" << request.rangeEnd() - 1;
    if(!request.cookies().empty())
        out << ", cookies=" << request.cookies() << "]";
    return out;
}

std::string escapeUrl(const std::string & url)
{
    char * temp = curl_easy_escape(0, url.c_str(), static_cast<int>(url.length()));
    if(temp)
    {
        std::string result(temp);
        curl_free(temp);
        return result;
    } else
        return std::string();
}

std::string unescapeUrl(const std::string & url)
{
    int outlen;
    char * temp = curl_easy_unescape(0, url.c_str(), static_cast<int>(url.length()), &outlen);
    if(temp)
    {
        std::string result(temp, outlen);
        curl_free(temp);
        return result;
    } else
        return std::string();
}

}
