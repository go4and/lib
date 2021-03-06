/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef NEXUS_BUILDING
#include <mstd/enum_utils.hpp>
#endif

#include "Buffer.h"

namespace nexus {

class RESTRequest {
public:
    RESTRequest(const std::string & base);

    void addString(const std::string & value) { str_ += value; }
    void setString(const std::string & value) { str_ = value; }
    void addParam(const std::string & param, const std::string & value);
    void addParam(const std::string & param, const char * value);
    void addParam(const char * param, const std::string & value);
    void addParam(const char * param, const char * value);
    const std::string & url(const std::string & apiSecret);
    const std::string & url(const char * secret);

    const std::string & url() { return url_; }
private:
    std::string url_;
    std::string str_;
};

}
