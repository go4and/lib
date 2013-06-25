/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
if !defined(_STLP_NO_IOSTREAMS)

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include <boost/thread/thread.hpp>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include "atomic_pod.hpp"

namespace mstd {

void atomic_pod_base::yield() const
{
    boost::this_thread::yield();
}

}

#endif
