#pragma once

#include <boost/config.hpp>

#if BOOST_WINDOWS
#include <Windows.h>

extern "C" void _mm_pause();
#pragma intrinsic( _mm_pause )
#else
#include <sched.h>
#include <time.h>
#endif

namespace mstd {

inline void yield(size_t k)
{
    if( k < 4 ) ;
#if BOOST_WINDOWS
    else if( k < 16 )
        _mm_pause();
    else if(k < 32)
        Sleep(0);
    else
        Sleep(1);
#else
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
#endif
}

}
