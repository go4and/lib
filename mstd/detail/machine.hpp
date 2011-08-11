#pragma once

#include <boost/cstdint.hpp>

#include <boost/spirit/home/phoenix/core/argument.hpp>
#include <boost/spirit/home/phoenix/core/value.hpp>
#include <boost/spirit/home/phoenix/operator/arithmetic.hpp>

#define MSTD_MACHINE_COMMON_PROCESSING

#include "waiter.hpp"

#include "machine_fwd.hpp"

#if _WIN32||_WIN64

#if defined(_M_IX86)
#include "windows32.hpp"
#else
#error Unsupported windows platform
#endif

#elif __linux__ || __FreeBSD__ || __APPLE__

#if __i386__
#include "linux32.hpp"
#elif __x86_64__
#include "linux64.hpp"
#endif

#else

#error Unsupported platform

#endif

namespace mstd { namespace detail {

inline void waiter::wait()
{
    if(state_ <= limit_)
    {
        detail::pause(state_);
        state_ += state_;
    } else
        detail::yield();
}

template<size_t S, class F>
inline typename size_to_int<S>::type generic_modify(volatile void *ptr, F f)
{
    typedef typename size_to_int<S>::type value_type;
    waiter b;
    value_type result;
    while(true)
    {
        result = *static_cast<volatile value_type*>(ptr);
        if(atomic_cas<S>(ptr, f(result), result) == result) 
            break;
        b.wait();
    }
    return result;
}

template<size_t size>
inline typename size_to_int<size>::type atomic_add(volatile void * ptr, typename size_to_int<size>::type value)
{
    return generic_modify<size>(ptr, boost::phoenix::arg_names::arg1 + value);
}

template<size_t size>
inline typename size_to_int<size>::type atomic_read_write(volatile void * ptr, typename size_to_int<size>::type value)
{
    return generic_modify<size>(ptr, boost::phoenix::val(value));
}

#undef MSTD_MACHINE_COMMON_PROCESSING

} }
