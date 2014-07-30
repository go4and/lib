/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#if !defined(_STLP_NO_IOSTREAMS)

#if defined(_MSC_VER)
#pragma warning(disable: 4512)
#endif

#include <boost/exception/diagnostic_information.hpp>
#endif

#include "exception.hpp"

namespace mstd {

#if !defined(_STLP_NO_IOSTREAMS)

void BoostExceptionOut::out(std::ostream & stream) const
{
    stream << mstd::get_error_message(*exc_) << " (" << diagnostic_information(*exc_) << ")";
}

void BoostExceptionOut::out(std::wostream & stream) const
{
    stream << mstd::wget_error_message(*exc_) << " (" << mstd::deutf8(diagnostic_information(*exc_)) << ")";
}

void StdExceptionOut::out(std::ostream &stream) const
{
    stream << exc_->what();
}

#endif

error_message make_error_message(const std::string & text)
{
    return error_message(text);
}

error_message make_error_message(const std::wstring & text)
{
    return error_message(mstd::utf8(text));
}

}
