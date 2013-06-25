/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
ifndef MSTD_MACHINE_COMMON_PROCESSING
#error Dont include this file directly, use common.hpp
#endif

#include <Windows.h>

extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)

namespace mstd { namespace detail {

inline void memory_fence() { _ReadWriteBarrier(); }

template<size_t Size>
struct atomic_helper;

template<>
struct atomic_helper<4> {
    typedef size_to_int<4>::type int_type;

    static int_type add(volatile int_type * ptr, int_type value)
    {
        return InterlockedExchangeAdd(ptr, value);
    }

    static int_type cas(volatile int_type * ptr, int_type newval, int_type oldval)
    {
        return InterlockedCompareExchange(ptr, newval, oldval);
    }

    static int_type read_write(volatile int_type * ptr, int_type value)
    {
        return InterlockedExchange(ptr, value);
    }
};

template<>
struct atomic_helper<8> {
    typedef size_to_int<8>::type int_type;

    static inline int_type add(volatile int_type * ptr, int_type value)
    {
        return InterlockedExchangeAdd(ptr, value);
    }

    static inline int_type cas(volatile int_type * ptr, int_type newval, int_type oldval)
    {
        return InterlockedCompareExchange(ptr, newval, oldval);
    }

    static inline int_type read_write(volatile int_type * ptr, int_type value)
    {
        return InterlockedExchange(ptr, value);
    }
};

template<size_t size>
inline typename size_to_int<size>::type atomic_read(const volatile void * ptr)
{
    typedef size_to_int<size>::type value_type;
    value_type result = *static_cast<const volatile value_type*>(ptr);
    memory_fence();
    return result;
}

template<size_t size>
inline void atomic_write(volatile void * ptr, typename size_to_int<size>::type value)
{
    memory_fence();
    *static_cast<volatile typename size_to_int<size>::type*>(ptr) = value;
}

} }
