#ifndef MSTD_MACHINE_COMMON_PROCESSING
#error Dont include this file directly, use common.hpp
#endif

#include "linux_common.hpp"

namespace mstd { namespace detail {

MSTD_DEFINE_ATOMICS(1, "")
MSTD_DEFINE_ATOMICS(2, "")
MSTD_DEFINE_ATOMICS(4, "l")

#undef MSTD_DEFINE_ATOMICS

template<>
boost::uint64_t atomic_cas<8>(volatile void *ptr, boost::uint64_t value, boost::uint64_t cmp);

#define MSTD_DETAIL_GCC_WORKAROUND_CAS8

template<>
inline boost::uint64_t atomic_read<8>(const volatile void *aptr)
{
    volatile boost::uint64_t * ptr = static_cast<volatile boost::uint64_t*>(const_cast<volatile void*>(aptr));
    if(!(reinterpret_cast<uint32_t>(ptr) & 7))
    {
        boost::uint64_t result;
        __asm__ __volatile__ ( "fildq %1\n\t"
                               "fistpq %0" : "=m"(result) : "m"(*ptr), "m"(result) : "memory");
        return result;
    } else
        return atomic_cas<8>(ptr, 0, 0);
}

template<>
inline void atomic_write<8>(volatile void *ptr, boost::uint64_t value)
{
    if(!(reinterpret_cast<uint32_t>(ptr) & 7u))
    {
        __asm__ __volatile__ ( "fildq %1\n\t"
                               "fistpq (%2)" :  "=m"(*static_cast<volatile boost::uint64_t*>(ptr)) : "m"(value), "r"(ptr) : "memory" );
    } else
        generic_modify<8>(ptr, Set<uint64_t>(value));
}

} }
