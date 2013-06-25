/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>

#include <boost/type_traits/is_pod.hpp>

#include "config.hpp"

#include "atomic.hpp"

namespace mstd {

class MSTD_DECL atomic_pod_base : public boost::noncopyable {
public:
protected:
    void yield() const;
private:
};

template<class T>
class atomic_pod : private atomic_pod_base {
public:
#if defined(_MSC_VER)
    BOOST_STATIC_ASSERT((boost::is_pod<T>::value));
#endif

    explicit atomic_pod()
        : dirty_flag_(0) {}

    T inline get() const
    {
        while(true)
        {
            boost::uint32_t oldFlag = dirty_flag_;
            T result(value_);
            if(oldFlag != INVALID_DIRTY_FLAG && oldFlag == dirty_flag_)
                return result;
            else
                yield();
        }
    }

    void set(const T & t)
    {
        boost::uint32_t old = dirty_flag_;
        dirty_flag_ = INVALID_DIRTY_FLAG;
        value_ = t;
        dirty_flag_ = old + 2;
    }
private:
    static const boost::uint32_t INVALID_DIRTY_FLAG = 1;

    atomic<boost::uint32_t> dirty_flag_;
    T value_;
};

}
