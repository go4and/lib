/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "pch.h"

#include "REST.h"

MLOG_DECLARE_LOGGER(rest);

namespace nexus {

RESTRequest::RESTRequest(const std::string & base)
    : url_(base)
{
}

void RESTRequest::addParam(const std::string & param, const std::string & value)
{
    str_ += param;
    str_ += '=';
    str_ += value;
    url_ += param;
    url_ += '=';
    url_ += value;
    url_ += '&';
}

void RESTRequest::addParam(const std::string & param, const char * value)
{
    str_ += param;
    str_ += '=';
    str_ += value;
    url_ += param;
    url_ += '=';
    url_ += value;
    url_ += '&';
}

void RESTRequest::addParam(const char * param, const std::string & value)
{
    str_ += param;
    str_ += '=';
    str_ += value;
    url_ += param;
    url_ += '=';
    url_ += value;
    url_ += '&';
}

void RESTRequest::addParam(const char * param, const char * value)
{
    str_ += param;
    str_ += '=';
    str_ += value;
    url_ += param;
    url_ += '=';
    url_ += value;
    url_ += '&';
}

const std::string & RESTRequest::url(const std::string & apiSecret)
{
    str_ += apiSecret;
    std::string sig = mcrypt::visualize(mcrypt::md5String(str_));
    url_ += "sig=";
    url_ += sig;

    return url_;
}

const std::string & RESTRequest::url(const char * apiSecret)
{
    str_ += apiSecret;
    std::string sig = mcrypt::visualize(mcrypt::md5String(str_));
    url_ += "sig=";
    url_ += sig;

    return url_;
}

}
