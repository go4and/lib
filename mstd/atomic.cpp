#include <boost/config.hpp>

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
