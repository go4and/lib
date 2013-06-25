/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#ifndef MSTD_MACHINE_COMMON_PROCESSING
#error Dont include this file directly, use machine.hpp
#endif

extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)

namespace mstd { namespace detail {
    #include "windows32_asm.hpp"

inline void memory_fence() { _ReadWriteBarrier(); }

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

#define MSTD_DETAIL_DEFINE_ATOMICS(SIZE, AREGISTER, CREGISTER) \
template<> \
inline size_to_int<SIZE>::type atomic_cas<SIZE>(volatile void * p, size_to_int<SIZE>::type value, size_to_int<SIZE>::type comparand) \
{ \
    size_to_int<SIZE>::type result; \
    __asm \
    { \
       __asm mov edx, p \
       __asm mov CREGISTER, value \
       __asm mov AREGISTER, comparand \
       __asm lock cmpxchg [edx], CREGISTER \
       __asm mov result, AREGISTER \
    } \
    return result; \
} \
\
template<> \
inline size_to_int<SIZE>::type atomic_add<SIZE>(volatile void * p, size_to_int<SIZE>::type value) \
{ \
    size_to_int<SIZE>::type result; \
    __asm { \
        __asm mov edx, p \
        __asm mov AREGISTER, value \
        __asm lock xadd [edx], AREGISTER \
        __asm mov result, AREGISTER \
    } \
    return result; \
}\
\
template<> \
inline size_to_int<SIZE>::type atomic_read_write<SIZE>(volatile void * p, size_to_int<SIZE>::type value) \
{ \
    size_to_int<SIZE>::type result; \
    __asm \
    { \
        __asm mov edx, p \
        __asm mov AREGISTER, value \
        __asm lock xchg [edx], AREGISTER \
        __asm mov result, AREGISTER \
    } \
    return result; \
}

MSTD_DETAIL_DEFINE_ATOMICS(1, al, cl)
MSTD_DETAIL_DEFINE_ATOMICS(2, ax, cx)
MSTD_DETAIL_DEFINE_ATOMICS(4, eax, ecx)

#undef MSTD_DETAIL_DEFINE_ATOMICS

void yield();

inline void pause(boost::uint32_t delay)
{
    __asm {
        mov eax, delay
      mloop: 
        pause
        add eax, -1
        jne mloop  
    }
}

} }
