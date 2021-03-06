/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/move/move.hpp>

#include "atomic.hpp"

namespace mstd {

class rc_array_base {
public:
    struct uninitialized_type {};
    static const uninitialized_type uninitialized;
};

template<class T>
class rc_array : public rc_array_base {
    BOOST_COPYABLE_AND_MOVABLE(rc_array);
public:
    typedef T value_type;
    
    rc_array()
        : data_(0)
    {
    }

    explicit rc_array(size_t size)
        : data_(new char[size * sizeof(value_type) + header_size])
    {
        *sizeAddress() = size;
        *counterAddress() = 1;
        std::uninitialized_fill(data(), data() + size, value_type());
    }

    explicit rc_array(size_t size, uninitialized_type raw)
        : data_(new char[size * sizeof(value_type) + header_size])
    {
        *sizeAddress() = size;
        *counterAddress() = 1;
    }

    explicit rc_array(const value_type * data, size_t size)
        : data_(new char[size * sizeof(value_type) + header_size])
    {
        *sizeAddress() = size;
        *counterAddress() = 1;
        std::uninitialized_copy(data, data + size, this->data());
    }

    template<class It>
    explicit rc_array(const It & begin, const It & end)
    {
        size_t size = std::distance(begin, end);
        data_ = new char[size * sizeof(value_type) + header_size];
        *sizeAddress() = size;
        *counterAddress() = 1;
        std::uninitialized_copy(begin, end, this->data());
    }

    ~rc_array()
    {
        reset();
    }

    rc_array(const rc_array & rhs)
        : data_(rhs.data_)
    {
        if(data_)
            detail::atomic_helper<sizeof(int)>::add(counterAddress(), 1);
    }

    void operator=(BOOST_COPY_ASSIGN_REF(rc_array) rhs)
    {
        reset();
        data_ = rhs.data_;
        if(data_)
            detail::atomic_helper<sizeof(int)>::add(counterAddress(), 1);
    }

    rc_array(BOOST_RV_REF(rc_array) rhs)
      : data_(rhs.data_)
    {
        rhs.data_ = 0;
    }

    void operator=(BOOST_RV_REF(rc_array) rhs)
    {
        if (this != &rhs)
        {
            reset();
            data_ = rhs.data_;
            rhs.data_ = 0;
        }
    }

    inline size_t size() const
    {
        return *sizeAddress();
    }

    inline value_type * data() const
    {
        return static_cast<value_type*>(static_cast<void*>(data_ + header_size));
    }

    inline value_type * begin() const
    {
        return data();
    }

    inline value_type * end() const
    {
        return begin() + size();
    }

    inline value_type & operator[](size_t index) const
    {
        return data()[index];
    }

    void reset()
    {
        if(data_)
        {
            typedef detail::atomic_helper<sizeof(int)> helper;
            if(helper::add(counterAddress(), static_cast<helper::int_type>(-1)) == 1)
            {
                size_t size = this->size();
                value_type * data = this->data();
                for(size_t i = size; i-- > 0;)
                    (data + i)->~value_type();
                delete [] data_;
            }
            data_ = 0;
        }
    }

    typedef char * rc_array::*unspecified_bool_type;

    operator unspecified_bool_type() const
    {
        return data_ == 0 ? 0: &rc_array::data_;
    }

    bool operator!() const
    {
        return data_ == 0;
    }

    typedef unsigned int counter_t;
    static const size_t min_header_size = sizeof(counter_t) + sizeof(size_t);
    static const size_t header_size = (min_header_size + sizeof(value_type) - 1) / sizeof(value_type) * sizeof(value_type);
private:
    size_t * sizeAddress() const
    {
        return static_cast<size_t*>(static_cast<void*>(data_));
    }

    volatile counter_t * counterAddress() const
    {
        return static_cast<volatile unsigned int*>(static_cast<void*>(data_ + sizeof(size_t)));
    }

    char * data_;
};

}
