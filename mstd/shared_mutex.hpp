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
#include <boost/thread/shared_mutex.hpp>
#else
#include <pthread.h>
#endif

namespace mstd {

#if 1
typedef boost::shared_mutex shared_mutex;
#else
class shared_mutex {
public:
    typedef boost::unique_lock<shared_mutex> unique_lock;
    typedef boost::shared_lock<shared_mutex> shared_lock;

    shared_mutex(void)
    {
        BOOST_VERIFY(!pthread_rwlock_init(&rwlock, NULL));
    }

    ~shared_mutex(void)
    {
        BOOST_VERIFY(!pthread_rwlock_destroy(&rwlock));
    }

    void lock(void)
    {
        BOOST_VERIFY(!pthread_rwlock_wrlock(&rwlock));
    }

    bool try_lock(void)
    {
        return analyze(pthread_rwlock_trywrlock(&rwlock));
    }

    void unlock(void)
    {
        BOOST_ASSERT(!pthread_rwlock_unlock(&rwlock));
    }

    void lock_shared(void)
    {
        BOOST_VERIFY(!pthread_rwlock_rdlock(&rwlock));
    }

    bool try_lock_shared(void)
    {
        return analyze(pthread_rwlock_tryrdlock(&rwlock));
    }

    void unlock_shared(void)
    {
        unlock();
    }
private:
    bool analyze(int status)
    {
        switch (status) {
        case 0:
            return true;
        case EBUSY:
            return false;
        case EDEADLK:
        default:
            BOOST_ASSERT(false);
        }
    }

    pthread_rwlock_t rwlock;
};
#endif

}
