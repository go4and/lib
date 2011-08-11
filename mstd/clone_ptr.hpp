#pragma once

#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>
#include <boost/detail/workaround.hpp>

#include <memory>

namespace mstd {

template<class T> class clone_ptr {
private:
    T * ptr;

    typedef clone_ptr<T> this_type;

    void operator==( clone_ptr const& ) const;
    void operator!=( clone_ptr const& ) const;

public:
    clone_ptr(clone_ptr const & rhs)
        : ptr(rhs.ptr ? new T(*rhs.ptr) : 0) {}

    clone_ptr & operator=(clone_ptr const & rhs)
    {
        reset(rhs.ptr ? new T(*rhs.ptr) : 0);
        return *this;
    }

    typedef T element_type;

    explicit clone_ptr(T * p = 0): ptr(p) // never throws
    {
    }

    explicit clone_ptr(std::auto_ptr<T> p): ptr(p.release()) // never throws
    {
    }

    ~clone_ptr() // never throws
    {
        boost::checked_delete(ptr);
    }

    void reset(T * p = 0) // never throws
    {
        BOOST_ASSERT(p == 0 || p != ptr); // catch self-reset errors
        this_type(p).swap(*this);
    }

    T & operator*() const // never throws
    {
        BOOST_ASSERT(ptr != 0);
        return *ptr;
    }

    T * operator->() const // never throws
    {
        BOOST_ASSERT(ptr != 0);
        return ptr;
    }

    T * get() const // never throws
    {
        return ptr;
    }

    // implicit conversion to "bool"

#if defined(__SUNPRO_CC) && BOOST_WORKAROUND(__SUNPRO_CC, <= 0x530)

    operator bool () const
    {
        return ptr != 0;
    }

#elif defined(__MWERKS__) && BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3003))
    typedef T * (this_type::*unspecified_bool_type)() const;
    
    operator unspecified_bool_type() const // never throws
    {
        return ptr == 0? 0: &this_type::get;
    }

#else 
    typedef T * this_type::*unspecified_bool_type;

    operator unspecified_bool_type() const // never throws
    {
        return ptr == 0? 0: &this_type::ptr;
    }

#endif

    bool operator! () const // never throws
    {
        return ptr == 0;
    }

    void swap(clone_ptr & b) // never throws
    {
        T * tmp = b.ptr;
        b.ptr = ptr;
        ptr = tmp;
    }
};

template<class T> inline void swap(clone_ptr<T> & a, clone_ptr<T> & b) // never throws
{
    a.swap(b);
}

// get_pointer(p) is a generic way to say p.get()

template<class T> inline T * get_pointer(clone_ptr<T> const & p)
{
    return p.get();
}

}
