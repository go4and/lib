#pragma once

#ifndef MNET_BUILDING
#include <boost/property_tree/ptree.hpp>
#endif

#include "DownloadDefines.h"

typedef void CURL;

namespace mnet {

typedef boost::function<void(int, const boost::property_tree::ptree&)> AsyncXmlHandler;
typedef boost::function<void(int, const std::string &)> AsyncDataHandler;
typedef boost::function<void(int, const std::string &, const std::string &)> AsyncDataExHandler;
typedef boost::function<void(int)> AsyncHandler;
typedef boost::function<void(size_t)> ProgressHandler;
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
void getXmlAsync(const std::string & url, const AsyncXmlHandler & handler, const std::string & cookie = std::string());
void getJSONAsync(const std::string & url, const AsyncXmlHandler & handler, const std::string & cookie = std::string());
void getDataAsync(const std::string & url, const AsyncDataHandler & handler, const std::string & cookie = std::string());
void getDataExAsync(const std::string & url, const AsyncDataExHandler & handler, const std::string & cookie = std::string());

int getRemoteFileSizeEx(std::string & uri, filesize_t & out);

}
