/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#if defined(_MSC_VER)
#pragma warning(disable: 4244)
#endif

#if !defined(_STLP_NO_IOSTREAMS)

#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/functional/hash/hash.hpp>

#include "threads.hpp"
#include "random.hpp"

namespace mstd {

boost::mt19937::result_type seed()
{
    using namespace boost::posix_time;
    time_duration::tick_type tick = (microsec_clock::universal_time() - from_time_t(0)).total_milliseconds();
    thread_id id = this_thread_id();
    return static_cast<boost::mt19937::result_type>(boost::hash_value(tick) ^ boost::hash_value(id));
}

}

#endif
