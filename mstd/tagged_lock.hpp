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
        : mutex_(mutex)
    {
        mutex.lock();
    }

    ~tagged_lock_base()
    {
        mutex_.unlock();
    }
private:
    Mutex & mutex_;
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
