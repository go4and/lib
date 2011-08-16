#pragma once

#ifndef NEXUS_BUILDING
#include <mstd/enum_utils.hpp>
#endif

#include "Buffer.h"

namespace nexus {

class RESTRequest {
public:
    RESTRequest(const std::string & base);

    void addParam(const std::string & param, const std::string & value);
    void addParam(const std::string & param, const char * value);
    void addParam(const char * param, const std::string & value);
    void addParam(const char * param, const char * value);
    const std::string & url(const std::string & apiSecret);
private:
    std::string url_;
    std::string str_;
};

MSTD_DEFINE_ENUM_EX(SocialNetwork, sn_, (vkont)(mailru));

const std::string & socialRequestBase(SocialNetwork type);

#ifndef __APPLE__
typedef boost::function<void(const boost::system::error_code &, const nexus::Buffer&)> URLHandler;
void getUrl(boost::asio::io_service & ios, const std::string & url, const URLHandler & handler);
#endif

}
