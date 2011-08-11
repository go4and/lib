#pragma once

#include "Buffer.h"

namespace nexus {

MSTD_DEFINE_ENUM_EX(SocialNetwork, sn_, (vkont)(mailru));

class SocialRequest {
public:
    SocialRequest(SocialNetwork type);

    void addParam(const std::string & param, const std::string & value);
    void addParam(const std::string & param, const char * value);
    void addParam(const char * param, const std::string & value);
    void addParam(const char * param, const char * value);
    const std::string & url(const std::string & apiSecret);
private:
    std::string url_;
    std::string str_;
};

typedef boost::function<void(const boost::system::error_code &, const nexus::Buffer&)> URLHandler;
void getUrl(boost::asio::io_service & ios, const std::string & url, const URLHandler & handler);

}
