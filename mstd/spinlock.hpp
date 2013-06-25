/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include "yield_k.hpp"

namespace mstd {

class spinlock {
public:
    spinlock()
        : v_(0)
    {
    }
public:
    bool try_lock()
    {
#if BOOST_WINDOWS
        long r = _InterlockedExchange(&v_, 1);
        _ReadWriteBarrier();
        return r == 0;
#else
        int r = __sync_lock_test_and_set(&v_, 1);
        return r == 0;
#endif
    }

    void lock()
    {
        for(size_t k = 0; !try_lock(); ++k)
            yield(k);
    }

    void unlock()
    {
#if BOOST_WINDOWS
        _ReadWriteBarrier();
        *const_cast< long volatile* >( &v_ ) = 0;
#else
        __sync_lock_release( &v_ );
#endif
    }
private:
#if BOOST_WINDOWS
    long v_;
#else
    int v_;
#endif
};

}
