/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#if !_STLP_NO_IOSTREAMS
#include <ostream>
#endif

#include "performance_timer.hpp"

namespace mstd {

#if !_STLP_NO_IOSTREAMS
std::ostream & operator<<(std::ostream & out, performance_interval i)
{
    return out << i.microseconds();
}

std::ostream & operator<<(std::ostream & out, performance_mark m)
{
#ifdef BOOST_WINDOWS
    return out << '<' << m.value_.QuadPart << '>';
#else
    return out << '<' << m.value_.tv_sec << ", " << m.value_.tv_nsec << '>';
#endif
}
#endif

}
