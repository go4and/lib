/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef MNET_BUILDING
#include <boost/intrusive_ptr.hpp>

#include <boost/filesystem/path.hpp>

#include <boost/property_tree/ptree.hpp>

#include <mstd/pointer_cast.hpp>
#include <mstd/rc_buffer.hpp>

#include <boost/variant/variant.hpp>
#endif

#include "DownloadDefines.h"

typedef void CURL;

namespace mnet {

typedef std::function<void(int ec, const boost::property_tree::ptree & tree)> AsyncPTreeHandler;
typedef std::function<void(int ec, const mstd::rc_buffer & data)> AsyncDataHandler;
typedef std::function<void(int ec, const mstd::rc_buffer & data, const mstd::rc_buffer & headers)> AsyncDataExHandler;
typedef std::function<void(int ec, filesize_t size)> AsyncSizeHandler;
typedef std::function<size_t(const char * buf, size_t size)> DirectWriter;
typedef std::function<void(int ec)> AsyncHandler;
typedef std::function<void(int percent)> ProgressHandler;
typedef std::function<void()> Action;
typedef std::function<void(const Action&)> UIEnqueuer;

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

typedef boost::variant<AsyncHandler, AsyncDataHandler, AsyncDataExHandler, AsyncSizeHandler> AsyncRequestHandler;

class Request {
public:
    Request()
        : rangeBegin_(-1) {}

    Request & url(const std::string & value) { url_ = value; return *this; }
    Request & cookies(const std::string & value) { cookies_ = value; return *this; }
    Request & progressHandler(const ProgressHandler & value) { progress_ = value; return *this; }
    Request & dataHandler(const AsyncDataHandler & value) { handler_ = value;; return *this; }
    Request & dataExHandler(const AsyncDataExHandler & value) { handler_ = value; return *this; }
    Request & sizeHandler(const AsyncSizeHandler & value) { handler_ = value; return *this; }
    Request & xmlHandler(const AsyncPTreeHandler & value);
    Request & jsonHandler(const AsyncPTreeHandler & value);
    Request & directWriter(const DirectWriter & writer, const AsyncHandler & handler) { directWriter_ = writer; handler_ = handler; return *this; }
    Request & postData(const void * data, size_t len) { postData_ = mstd::rc_buffer(static_cast<const char*>(data), len); return *this; }
    Request & postData(const std::string & data) { return postData(data.c_str(), data.size()); }
    Request & postData(const mstd::rc_buffer & data) { postData_ = data; return *this; }
    Request & header(const std::string & line) { headers_.push_back(line); return *this; }
    Request & range(filesize_t begin, filesize_t end) { rangeBegin_ = begin; rangeEnd_ = end; return *this; }

    const std::string & url() const { return url_; }
    const std::string & cookies() const { return cookies_; }
    const ProgressHandler & progressHandler() const { return progress_; }
    const AsyncRequestHandler & handler() const { return handler_; }
    const DirectWriter & directWriter() const { return directWriter_; }
    const mstd::rc_buffer & postData() const { return postData_; }
    const std::vector<std::string> & headers() const { return headers_; }
    filesize_t rangeBegin() const { return rangeBegin_; }
    filesize_t rangeEnd() const { return rangeEnd_; }

    void run();
private:
    std::string url_;
    std::string cookies_;
    std::vector<std::string> headers_;
    ProgressHandler progress_;
    AsyncRequestHandler handler_;
    DirectWriter directWriter_;
    filesize_t rangeBegin_, rangeEnd_;
    mstd::rc_buffer postData_;
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
std::string unescapeUrl(const std::string & url);

}
