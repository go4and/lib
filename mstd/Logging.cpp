/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#if !defined(_STLP_NO_IOSTREAMS)

#include "strings.hpp"

#include "Logging.h"

using namespace std;

namespace mstd {

LogStream::LogStream(const LogDevice & device, LogLevel level)
    : device_(device), level_(level) {}

wostream & LogStream::impl()
{
    return stream_;
}

LogStream::~LogStream()
{
    device_(stream_.str(), level_);
}

LogStream & LogStream::operator<<(LogStream & (*func)(LogStream &))
{
    return func(*this);
}

LogStream & LogStream::operator<<(const std::string & str)
{
    stream_ << mstd::deutf8(str);
    return *this;
}

LogStream & endl(LogStream & stream)
{
    stream.impl() << std::endl;
    return stream;
}

}

#endif
