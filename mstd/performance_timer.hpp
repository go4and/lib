/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <boost/config.hpp>

#ifdef BOOST_WINDOWS
#include <Windows.h>
#else
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#endif

#if !_STLP_NO_IOSTREAMS
#include <iosfwd>
#endif

#include <boost/mpl/integral_c.hpp>

#include "cstdint.hpp"

namespace mstd {

typedef int64_t performance_value_type;

class performance_interval {
public:
    explicit performance_interval(performance_value_type value = 0)
        : value_(value) {}

    inline int64_t milliseconds()
    {
        return get<boost::mpl::integral_c<int64_t, 1000> >();
    }

    inline int64_t microseconds()
    {
        return get<boost::mpl::integral_c<int64_t, 1000000> >();
    }

    inline int64_t nanoseconds()
    {
#ifdef BOOST_WINDOWS
        return value_ * 100;
#else
        return value_;
#endif    
    }

    performance_interval & operator+=(performance_interval rhs)
    {
        value_ += rhs.value_;
        return *this;
    }
private:
    performance_value_type value_;

    template<class PS>
    inline int64_t get()
    {
#ifdef BOOST_WINDOWS
        return value_ / boost::mpl::integral_c<int64_t, 10000000 / PS::value>::value;
#else
        return value_ / boost::mpl::integral_c<int64_t, 1000000000 / PS::value>::value;
#endif    
    }
};

#if !_STLP_NO_IOSTREAMS
std::ostream & operator<<(std::ostream & out, performance_interval i);
#endif

class performance_mark {
public:
    performance_mark()
    {
        reset();
    }

    void reset()
    {
#ifdef BOOST_WINDOWS
        FILETIME t1, t2, t3, t4;
        GetThreadTimes(GetCurrentThread(), &t1, &t2, &t3, &t4);
        value_.LowPart = t3.dwLowDateTime;
        value_.HighPart = t3.dwHighDateTime;
        LARGE_INTEGER t;
        t.LowPart = t4.dwLowDateTime;
        t.HighPart = t4.dwHighDateTime;
        value_.QuadPart += t.QuadPart;
#elif defined(CLOCK_THREAD_CPUTIME_ID)
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &value_);
#else
        value_.tv_sec = value_.tv_nsec = 0;
#endif
    }

    friend inline performance_interval operator-(const performance_mark & lhs, const performance_mark & rhs)
    {
#ifdef BOOST_WINDOWS
        return performance_interval((lhs.value_.QuadPart - rhs.value_.QuadPart));
#else
        return performance_interval((lhs.value_.tv_sec - rhs.value_.tv_sec) * 1000000000LL + (lhs.value_.tv_nsec - rhs.value_.tv_nsec));
#endif    
    }
private:
#ifdef BOOST_WINDOWS
    LARGE_INTEGER value_;
#else
    timespec value_;
#endif    

    friend bool operator!=(performance_mark lhs, performance_mark rhs)
    {
#ifdef BOOST_WINDOWS
        return lhs.value_.QuadPart != rhs.value_.QuadPart;
#else
        return lhs.value_.tv_nsec != rhs.value_.tv_nsec || lhs.value_.tv_sec != rhs.value_.tv_sec;
#endif
    }

    friend bool operator==(performance_mark lhs, performance_mark rhs)
    {
        return !(lhs != rhs);
    }

#if !_STLP_NO_IOSTREAMS
    friend std::ostream & operator<<(std::ostream & out, performance_mark m);
#endif
};

#if !_STLP_NO_IOSTREAMS
std::ostream & operator<<(std::ostream & out, performance_mark m);
#endif

class performance_timer {
public:
    performance_timer()
    {
    }

    int64_t microseconds() const
    {
        performance_mark stop;
        return (stop - start_).microseconds();
    }
private:
    performance_mark start_;
};

}
