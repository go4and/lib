/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
#include <windows.h>
#endif

#include "atomic.hpp"

namespace mstd { namespace detail {

#if defined(BOOST_WINDOWS)
void yield()
{
    SwitchToThread();
}
#endif

} }
