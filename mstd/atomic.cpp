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

#if defined(MSTD_DETAIL_GCC_WORKAROUND_CAS8)

template<>
boost::uint64_t atomic_cas<8>(volatile void *ptr, boost::uint64_t value, boost::uint64_t acmp)
{
    boost::uint64_t result;
    union {
      boost::uint64_t cmp;
      boost::uint32_t cmps[2];
    };
    cmp = acmp;
    __asm__ __volatile__ (
             "pushl %%ebx\n\t"
             "movl  (%%ecx), %%ebx\n\t"
             "movl  4(%%ecx), %%ecx\n\t"
             "lock\ncmpxchg8b (%2)\n\t"
             "popl  %%ebx"
                    : "=A"(result), "=m"(*static_cast<volatile boost::uint64_t*>(ptr))
                    : "S"(ptr),
                      "a"(cmps[0]),
                      "d"(cmps[1]),
                      "c"(&value)
                    : "memory", "esp");
    return result;
}

#endif

} }
