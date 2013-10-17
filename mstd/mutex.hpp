/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/config.hpp>

#if BOOST_WINDOWS
#include <Windows.h>
#else
#include <pthread.h>
#endif

#include <boost/noncopyable.hpp>

namespace mstd {

class mutex : public boost::noncopyable {
#if BOOST_WINDOWS
public:
    mutex()
    {
        InitializeCriticalSection(&value_);
    }

    void lock()
    {
        EnterCriticalSection(&value_);
    }

    void unlock()
    {
        LeaveCriticalSection(&value_);
    }

    ~mutex()
    {
        DeleteCriticalSection(&value_);
    }
private:
    CRITICAL_SECTION value_;
#else
public:
    mutex()
    {
        pthread_mutex_init(&value_, 0);
    }

    void lock()
    {
        pthread_mutex_lock(&value_);
    }

    void unlock()
    {
        pthread_mutex_unlock(&value_);
    }

    ~mutex()
    {
        pthread_mutex_destroy(&value_);
    }
private:
    pthread_mutex_t value_;
#endif
};

template<class Mutex>
class lock_guard {
public:
    explicit lock_guard(Mutex & m)
        : mutex_(m)
    {
        m.lock();
    }

    ~lock_guard()
    {
        mutex_.unlock();
    }
private:
    lock_guard(lock_guard&);
    void operator=(lock_guard&);

    Mutex & mutex_;
};

template<class Mutex>
class unique_lock {
public:
    explicit unique_lock(Mutex & m)
        : mutex_(m), locked_(true)
    {
        mutex_.lock();
    }

    bool owns_lock()
    {
        return locked_;
    }

    ~unique_lock()
    {
        if(owns_lock())
            mutex_.unlock();
    }

    void lock()
    {
        mutex_.lock();
        locked_ = true;
    }

    void unlock()
    {
        mutex_.unlock();
        locked_ = false;
    }
private:
    bool locked_;
    Mutex & mutex_;
};

}
