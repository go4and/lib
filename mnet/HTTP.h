#pragma once

#ifndef MNET_BUILDING
#include <boost/function.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/filesystem/path.hpp>

#include <boost/property_tree/ptree.hpp>

#include <mstd/pointer_cast.hpp>
#include <mstd/reference_counter.hpp>
#endif

#include "DownloadDefines.h"

typedef void CURL;

namespace mnet {

typedef boost::function<void(int, const boost::property_tree::ptree&)> AsyncPTreeHandler;
typedef boost::function<void(int, const std::string &)> AsyncDataHandler;
typedef boost::function<void(int, const std::string &, const std::string &)> AsyncDataExHandler;
typedef boost::function<void(int)> AsyncHandler;
typedef boost::function<void(int /* percent */)> ProgressHandler;
typedef boost::function<void()> Action;
typedef boost::function<void(const Action&)> UIEnqueuer;

struct Proxy {
    bool active;
    bool useSystem;
    std::string host;
    int port;
    std::string login;
    std::string password;
};

void setUserAgent(const std::string & userAgent);
void setProxy(const Proxy & proxy);
void setUIEnqueuer(const UIEnqueuer & enqueuer);

const std::string & userAgent();

CURL * createCurl(const std::string & url, const std::string & cookies = std::string());

void getFileAsync(const std::string & url, const boost::filesystem::wpath & path, const AsyncHandler & handler);
void getFileAsync(const std::string & url, const boost::filesystem::wpath & path, const AsyncHandler & handler, const ProgressHandler& progress);

boost::property_tree::ptree parseXml(const std::string& data);
boost::property_tree::ptree parseJSON(const std::string& data);

class PostData : public mstd::reference_counter<PostData> {
public:
    explicit PostData(size_t len)
        : len_(len)
    {
    }

    void * data() { return mstd::pointer_cast<char*>(this) + sizeof(*this); }
    const void * data() const { return mstd::pointer_cast<const char*>(this) + sizeof(*this); }

    size_t size() const { return len_; }
private:
    size_t len_;
};

inline PostData * makePostData(size_t len)
{
    char * placeholder = new char[sizeof(PostData) + len];
    PostData * result = new (placeholder) PostData(len);
    return result;
}

inline PostData * makePostData(const void * input, size_t len)
{
    PostData * result = makePostData(len);
    memcpy(result->data(), input, len);
    return result;
}

typedef boost::intrusive_ptr<PostData> PostDataPtr;

class Request {
public:
    Request() {}

    Request & url(const std::string & value) { url_ = value; return *this; }
    Request & cookies(const std::string & value) { cookies_ = value; return *this; }
    Request & progressHandler(const ProgressHandler & value) { progress_ = value; return *this; }
    Request & dataHandler(const AsyncDataHandler & value) { handler_ = value; handlerEx_.clear(); return *this; }
    Request & dataExHandler(const AsyncDataExHandler & value) { handlerEx_ = value; handler_.clear(); return *this; }
    Request & xmlHandler(const AsyncPTreeHandler & value);
    Request & jsonHandler(const AsyncPTreeHandler & value);
    Request & postData(const void * data, size_t len) { return postData(makePostData(data, len)); }
    Request & postData(const PostDataPtr & data) { postData_ = data; return *this; }
    Request & header(const std::string & line) { headers_.push_back(line); return *this; }

    const std::string & url() const { return url_; }
    const std::string & cookies() const { return cookies_; }
    const ProgressHandler & progressHandler() const { return progress_; }
    const AsyncDataHandler & dataHandler() const { return handler_; }
    const AsyncDataExHandler & dataExHandler() const { return handlerEx_; }
    const PostDataPtr & postData() const { return postData_; }
    const std::vector<std::string> & headers() const { return headers_; }

    void run();
private:
    std::string url_;
    std::string cookies_;
    std::vector<std::string> headers_;
    ProgressHandler progress_;
    AsyncDataHandler handler_;
    AsyncDataExHandler handlerEx_;
    PostDataPtr postData_;
};

std::ostream & operator<<(std::ostream & out, const Request & request);

inline void getXmlAsync(const std::string & url, const AsyncPTreeHandler & handler, const std::string & cookies = std::string())
{
    Request().url(url).cookies(cookies).xmlHandler(handler).run();
}

inline void getJSONAsync(const std::string & url, const AsyncPTreeHandler & handler, const std::string & cookies = std::string())
{
    Request().url(url).cookies(cookies).jsonHandler(handler).run();
}

inline void getDataAsync(const std::string & url, const AsyncDataHandler & handler, const std::string & cookies = std::string())
{
    Request().url(url).cookies(cookies).dataHandler(handler).run();
}

inline void getDataAsync(const std::string & url, const AsyncDataHandler & handler, const ProgressHandler & progress, const std::string & cookies = std::string())
{
    Request().url(url).cookies(cookies).progressHandler(progress).dataHandler(handler).run();
}

inline void getDataExAsync(const std::string & url, const AsyncDataExHandler & handler, const std::string & cookies = std::string())
{
    Request().url(url).cookies(cookies).dataExHandler(handler).run();
}

void cancelAll();

int getRemoteFileSizeEx(std::string & uri, filesize_t & out);

std::string escapeUrl(const std::string & url);

}
