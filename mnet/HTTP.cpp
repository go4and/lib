/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#{
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
    mstd::rc_buffer blankBuffer_;
};

void AsyncTask::uiEnqueue(const boost::function<void()> & action)
{
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
        parseJSON(tree ,data);
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
    char * temp = curl_easy_escape(0, url.c_str(), url.length());
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
    char * temp = curl_easy_unescape(0, url.c_str(), url.length(), &outlen);
    if(temp)
    {
        std::string result(temp, outlen);
        curl_free(temp);
        return result;
    } else
        return std::string();
}

}
