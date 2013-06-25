/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <boost/cstdint.hpp>

#define MSTD_MACHINE_COMMON_PROCESSING

#include "waiter.hpp"
#include "../yield_k.hpp"

#include "machine_fwd.hpp"

#if _WIN32||_WIN64

#if defined(_M_IX86)
#include "windows32.hpp"
#elif defined(_M_X64)
#include "windows64.hpp"
#else
#error Unsupported Windows platform
#endif

#elif __linux__ || __FreeBSD__ || __APPLE__

#include "linux_common.hpp"

#else

#error Unsupported platform

#endif

namespace mstd { namespace detail {

inline void waiter::wait()
{
    yield(state_);
    ++state_;
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
        if(atomic_helper<S>::cas(static_cast<volatile value_type*>(ptr), f(result), result) == result) 
            break;
        b.wait();
    }
    return result;
}

template<class Value>
class Append {
public:
    explicit Append(const Value & v)
        : value_(v) {}

    template<class T>
    T operator()(const T & t) const
    {
        return t + value_;
    }
private:
    Value value_;
};

#undef MSTD_MACHINE_COMMON_PROCESSING

} }
