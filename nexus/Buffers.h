#pragma once

#ifndef NEXUS_BUILDING

#include <deque>

#include <boost/array.hpp>
#include <boost/asio/buffer.hpp>

#endif

#include "Buffer.h"

namespace nexus {

template<class Impl>
class BuffersIterator {
public:
    explicit BuffersIterator(const Impl & impl, size_t skip)
        : impl_(impl), skip_(skip) {}

    BuffersIterator & operator++()
    {
        ++impl_;
        skip_ = 0;
        return *this;
    }

    boost::asio::const_buffer operator*() const
    {
        return impl_->make(skip_);
    }
    
    bool equals(const BuffersIterator & rhs) const
    {
        return impl_ == rhs.impl_;
    }
private:
    Impl impl_;
    size_t skip_;
};

template<class Impl>
inline bool operator!=(const BuffersIterator<Impl> & lhs, const BuffersIterator<Impl> & rhs)
{
    return !lhs.equals(rhs);
}

class Buffers;

class NEXUS_DECL BuffersRef {
public:
    typedef boost::array<Buffer, 0x4> Value;
    typedef BuffersIterator<Value::const_iterator> const_iterator;

    explicit BuffersRef(const Buffers & source);

    const_iterator begin() const
    {
        return const_iterator(value_.begin(), skip_);
    }

    const_iterator end() const
    {
        return const_iterator(value_.begin() + size_, 0);
    }
private:
    Value value_;
    size_t skip_;
    size_t size_;
};

class NEXUS_DECL Buffers {
public:
    Buffers();

    void push_back(const Buffer & buffer)
    {
        total_ += buffer.size();
        buffers_.push_back(buffer);
    }

    bool empty() const
    {
        return buffers_.empty();
    }

    void clear()
    {
        buffers_.clear();
        total_ = 0;
        skip_ = 0;
    }

    size_t total() const
    {
        return total_;
    }

    template<class Container>
    void add(const Container & container)
    {
        buffers_.insert(buffers_.end(), container.begin(), container.end());
        for(typename Container::const_iterator i = container.begin(), end = container.end(); i != end; ++i)
            total_ += i->size();
    }

    void erase(size_t len);

    BuffersRef ref() const
    {
        return BuffersRef(*this);
    }

    bool mayAdd() const
    {
        return buffers_.size() < BuffersRef::Value::static_size;
    }
private:
    std::deque<Buffer> buffers_;
    size_t skip_;
    size_t total_;

    friend class BuffersRef;
};

class NEXUS_DECL SingleBuffer {
public:
    explicit SingleBuffer(const Buffer & buffer)
        : buffer_(buffer) {}

    class const_iterator : public std::iterator<std::forward_iterator_tag, boost::asio::const_buffer> {
    public:
        explicit const_iterator(const Buffer * buffer)
            : buffer_(buffer), end_(false) {}

        explicit const_iterator()
            : buffer_(0), end_(true) {}

        bool operator!=(const const_iterator & rhs) const
        {
            return end_ != rhs.end_;
        }

        bool operator==(const const_iterator & rhs) const
        {
            return end_ == rhs.end_;
        }

        const_iterator & operator++()
        {
            end_ = true;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator result(*this);
            ++*this;
            return result;
        }

        boost::asio::const_buffer operator*() const
        {
            return buffer_->make(0);
        }
    private:
        const Buffer * buffer_;
        bool end_;
    };

    const_iterator begin() const
    {
        return const_iterator(&buffer_);
    }

    const_iterator end() const
    {
        return const_iterator();
    }
private:
    const Buffer & buffer_;
};

}
