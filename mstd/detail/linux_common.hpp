#ifndef MSTD_MACHINE_COMMON_PROCESSING
#error Dont include this file directly, use common.hpp
#endif

#include <sched.h>

#define MSTD_DEFINE_ATOMICS(SIZE, SUFFIX) \
template<> \
inline size_to_int<SIZE>::type atomic_cas<SIZE>(volatile void* ptr, size_to_int<SIZE>::type value, size_to_int<SIZE>::type cmp) \
{ \
    typedef size_to_int<SIZE>::type value_type; \
    value_type result; \
    __asm__ __volatile__("lock\ncmpxchg" SUFFIX " %2, %1" \
                          : "=a"(result), "=m"(*static_cast<volatile value_type*>(ptr)) \
                          : "q"(value), "0"(cmp) \
                          : "memory"); \
    return result; \
} \
\
template<> \
inline size_to_int<SIZE>::type atomic_add<SIZE>(volatile void* ptr, size_to_int<SIZE>::type value) \
{ \
    typedef size_to_int<SIZE>::type value_type; \
    value_type result; \
    __asm__ __volatile__("lock\nxadd" SUFFIX " %0, %1" \
                          : "=r"(result), "=m"(*static_cast<volatile value_type*>(ptr)) \
                          : "0"(value) \
                          : "memory"); \
   return result; \
} \
\
template<> \
inline size_to_int<SIZE>::type atomic_read_write<SIZE>(volatile void* ptr, size_to_int<SIZE>::type value) \
{ \
    typedef size_to_int<SIZE>::type value_type; \
    value_type result; \
    __asm__ __volatile__("lock\nxchg" SUFFIX " %0, %1" \
                          : "=r"(result), "=m"(*static_cast<volatile value_type*>(ptr)) \
                          : "0"(value) \
                          : "memory"); \
   return result; \
}

namespace mstd { namespace detail {

inline void memory_fence() { __asm__ __volatile__("": : :"memory"); }

inline void yield()
{
    sched_yield();
}

inline void pause(boost::uint32_t delay)
{
    while(delay--)
       __asm__ __volatile__("pause;");
}

template<size_t size>
inline typename size_to_int<size>::type atomic_read(const volatile void * ptr)
{
    typedef typename size_to_int<size>::type value_type;
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
