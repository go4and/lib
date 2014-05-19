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

#include "config.hpp"

namespace boost {
    class mutex;
}

namespace mstd {

template<class Tag>
typename Tag::mutex_type & get_mutex(Tag & tag)
{
    return tag.mutex();
}

template<class Mutex>
class MSTD_DECL tagged_lock_base : public boost::noncopyable {
protected:
    explicit tagged_lock_base(Mutex & mutex)
        : mutex_(mutex), locked_(true)
    {
        mutex.lock();
    }

    ~tagged_lock_base()
    {
        if(locked_)
            mutex_.unlock();
    }
public:
    void unlock()
    {
        BOOST_ASSERT(locked_);
        mutex_.unlock();
        locked_ = false;
    }
    
    void lock()
    {
        BOOST_ASSERT(!locked_);
        mutex_.lock();
        locked_ = true;
    }
private:
    Mutex & mutex_;
    bool locked_;
};

template<class Tag, class Mutex = boost::mutex>
class tagged_lock : public tagged_lock_base<Mutex> {
public:
    typedef tagged_lock_base<Mutex> base_type;
    
    explicit tagged_lock(Tag & tag)
        : base_type(get_mutex(tag)) {}

#if defined(_MSV_VER)
private:
#endif
    explicit tagged_lock(Mutex & mutex)
        : base_type(mutex) {}

private:

#if defined(_MSC_VER)
    friend typename Tag;
#endif
};

}
