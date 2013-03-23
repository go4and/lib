#pragma once

namespace mstd {

namespace detail {

    inline void yield( size_t k )
    {
        if( k < 4 ) ;
        else if( k < 16 )
            __asm__ __volatile__( "rep; nop" : : : "memory" );
        else if( k < 32 || k & 1 )
            sched_yield();
        else {
            struct timespec rqtp;

            rqtp.tv_sec = 0;
            rqtp.tv_nsec = 1000;

            nanosleep( &rqtp, 0 );
        }
    }

}

class spinlock {
public:
    spinlock()
        : v_(0)
    {
    }
public:
    bool try_lock()
    {
        int r = __sync_lock_test_and_set(&v_, 1);
        return r == 0;
    }

    void lock()
    {
        for(size_t k = 0; !try_lock(); ++k)
            detail::yield(k);
    }

    void unlock()
    {
        __sync_lock_release( &v_ );
    }
private:
    int v_;
};

}
