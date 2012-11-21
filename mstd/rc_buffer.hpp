#pragma once

#include <string.h>

#include "atomic.hpp"

namespace mstd {

class rc_buffer {
public:
    rc_buffer()
        : data_(0)
    {
    }

    explicit rc_buffer(size_t size)
        : data_(new char[size + sizeof(counter_t) + sizeof(size_t)])
    {
        *sizeAddress() = size;
        *counterAddress() = 1;
    }

    rc_buffer(const char * data, size_t size)
        : data_(new char[size + sizeof(counter_t) + sizeof(size_t)])
    {
        memcpy(this->data(), data, size);
        *sizeAddress() = size;
        *counterAddress() = 1;
    }

    ~rc_buffer()
    {
        reset();
    }

    rc_buffer(const rc_buffer & rhs)
        : data_(rhs.data_)
    {
        if(data_)
            detail::atomic_helper<sizeof(int)>::add(counterAddress(), 1);
    }

    void operator=(const rc_buffer & rhs)
    {
        reset();
        data_ = rhs.data_;
        if(data_)
            detail::atomic_helper<sizeof(int)>::add(counterAddress(), 1);
    }

    size_t size() const
    {
        return *sizeAddress();
    }

    char * data() const
    {
        return data_ + sizeof(counter_t) + sizeof(size_t);
    }

    unsigned char * udata() const
    {
        return static_cast<unsigned char*>(static_cast<void*>(data()));
    }

    void reset()
    {
        if(data_)
        {
            typedef detail::atomic_helper<sizeof(int)> helper;
            helper::int_type result = helper::add(counterAddress(), static_cast<helper::int_type>(-1));
            if(result == 1)
                delete [] data_;
            data_ = 0;
        }
    }

    typedef char * rc_buffer::*unspecified_bool_type;

    operator unspecified_bool_type() const
    {
        return data_ == 0 ? 0: &rc_buffer::data_;
    }

    bool operator!() const
    {
        return data_ == 0;
    }
private:
    typedef unsigned int counter_t;

    size_t * sizeAddress() const
    {
        return static_cast<size_t*>(static_cast<void*>(data_ + sizeof(counter_t)));
    }

    volatile counter_t * counterAddress() const
    {
        return static_cast<volatile unsigned int*>(static_cast<void*>(data_));
    }

    char * data_;
};

}
