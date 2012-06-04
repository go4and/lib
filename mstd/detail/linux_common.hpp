#ifndef MSTD_MACHINE_COMMON_PROCESSING
#error Dont include this file directly, use common.hpp
#endif

#include <sched.h>

namespace mstd { namespace detail {

template<size_t Size>
struct atomic_helper {
    typedef typename size_to_int<Size>::type int_type;

    static int_type add(volatile int_type * ptr, int_type value)
    {
        return __sync_fetch_and_add(ptr, value);
    }

    static int_type cas(volatile int_type * ptr, int_type newval, int_type oldval)
    {
        return __sync_val_compare_and_swap(ptr, oldval, newval);
    }

    static int_type read_write(volatile int_type * ptr, int_type value)
    {
        return __sync_lock_test_and_set(ptr, value);
    }
};

inline void memory_fence() { __sync_synchronize(); }

inline void yield()
{
    sched_yield();
}

inline void pause(boost::uint32_t delay)
{
    while(delay--)
       __sync_synchronize();
}

template<size_t size>
inline typename size_to_int<size>::type atomic_read(const volatile void * ptr)
{
    typedef typename size_to_int<size>::type value_type;
    value_type result = *static_cast<const volatile value_type*>(ptr);
    __sync_synchronize();
    return result;
}

template<size_t size>
inline void atomic_write(volatile void * ptr, typename size_to_int<size>::type value)
{
    __sync_synchronize();
    *static_cast<volatile typename size_to_int<size>::type*>(ptr) = value;
}

} }
