/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "src/pch.hpp"

#include "writers.hpp"

namespace mptree {

size_t render_short_value(char * out, double value)
{
#if !BOOST_WINDOWS
    return snprintf(out, 0x40 - 1, "%g", value);
#else
    return _snprintf(out, 0x40 - 1, "%g", value);
#endif
}

}
