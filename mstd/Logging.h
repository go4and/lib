/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <string>
#include <sstream>

#include <boost/function.hpp>

#include "config.hpp"

namespace mstd {

enum LogLevel {
    llFatal,
    llError,
    llWarn,
    llInfo,
    llNotice,
};

typedef boost::function<void(const std::wstring &, LogLevel)> LogDevice;

class MSTD_DECL LogStream {
public:
    LogStream(const LogDevice & device, LogLevel level);
    ~LogStream();

    std::wostream & impl();
    
    template<class T>
    LogStream & operator<<(const T & t)
    {
        stream_ << t;
        return *this;
    }

    LogStream & operator<<(const std::string & str);
    LogStream & operator<<(LogStream & (*func)(LogStream &));
private:
    std::wostringstream stream_;
    const LogDevice   & device_;
    LogLevel            level_;
};

MSTD_DECL LogStream & endl(LogStream & stream);

}
