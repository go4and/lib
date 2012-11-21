#pragma once

#include "atomic.hpp"

namespace mstd {

template<class T>
class rc_array {
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
        std::uninitialized_fill_n(data(), size, value_type());
    }

    explicit rc_array(const value_type * data, size_t size)
        : data_(new char[size * sizeof(value_type) + header_size])
    {
        *sizeAddress() = size;
        *counterAddress() = 1;
        std::uninitialized_copy_n(data, size, this->data());
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

    void operator=(const rc_array & rhs)
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

    value_type * data() const
    {
        return static_cast<value_type*>(static_cast<void*>(data_ + header_size));
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
