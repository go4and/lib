#pragma once

#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace mstd {

namespace detail {

template<class Mutex>
class nolock {
public:
    explicit nolock(Mutex & mutex) {}
    bool owns_lock() const { return true; }
};

}

template<class T, template<class M> class Lock, class Ret>
class access_base;

template<class T>
class access_source : public boost::noncopyable {
public:
    explicit access_source(T & t, boost::shared_mutex & mutex)
        : t_(&t), mutex_(mutex) {}
private:
    T * t_;
    boost::shared_mutex & mutex_;
    
    template<class T, template<class M> class Lock, class Ret>
    friend class access_base;
};

template<class T, bool writable>
class access_source_ref {
public:
    explicit access_source_ref(const access_source<T> & source)
        : source_(&source) {}

    access_source_ref(const access_source_ref<T, true> & ref)
        : source_(&ref.source()) {}

    const access_source<T> & source() const
    {
        return *source_;
    }
private:
    const access_source<T> * source_;
};

template<class T, template<class M> class Lock, class Ret>
class access_base {
public:
    template<bool w>
    explicit access_base(const access_source_ref<T, w> & ref)
        : lock_(ref.source().mutex_), t_(ref.source().t_) {}

    Ret & get() const
    {
        BOOST_ASSERT(lock_.owns_lock());
        return *t_;
    }

    Ret * operator->() const
    {
        BOOST_ASSERT(lock_.owns_lock());
        return t_;
    }

    void unlock()
    {
        lock_.unlock();
    }
private:
    Lock<boost::shared_mutex> lock_;
    Ret * t_;
};

template<class T>
class access_nolock : public access_base<T, detail::nolock, const T> {
public:
    template<bool w>
    explicit access_nolock(const access_source_ref<T, w> & ref)
        : access_base(ref)
    {
    }
};

template<class T>
class access_read : public access_base<T, boost::shared_lock, const T> {
public:
    template<bool w>
    explicit access_read(const access_source_ref<T, w> & ref)
        : access_base(ref)
    {
    }
};

template<class T>
class access_write : public access_base<T, boost::unique_lock, T> {
public:
    explicit access_write(const access_source_ref<T, true> & ref)
        : access_base(ref)
    {
    }
};

template<class T, bool w>
access_nolock<T> nolock(const access_source_ref<T, w> & ref)
{
    return access_nolock<T>(ref);
}

template<class T, bool w>
access_read<T> read(const access_source_ref<T, w> & ref)
{
    return access_read<T>(ref);
}

template<class T>
access_write<T> write(const access_source_ref<T, true> & ref)
{
    return access_write<T>(ref);
}

}
