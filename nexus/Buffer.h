/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#ifndef NEXUS_BUILDING

#include <mstd/buffers.hpp>

#endif

#include "Config.h"

namespace boost { namespace asio {
    class const_buffer;    
} }

namespace nexus {

class NEXUS_DECL Buffer {
public:
    Buffer() {}
    explicit Buffer(size_t len);
    Buffer(const char * data, size_t len);
    Buffer(const char * data, const char * end);

    template<class Allocator>
    Buffer(size_t len, Allocator & allocator)
        : buffer_(allocator(len + sizeof(size_t)))
    {
        *size_ptr() = len;
    }

    template<class Allocator>
    Buffer(const char * source, size_t len, Allocator & allocator)
        : buffer_(allocator(len + sizeof(size_t)))
    {
        *size_ptr() = len;
        memcpy(data(), source, len);
    }

    char * data()
    {
        return buffer_->ptr() + sizeof(size_t);
    }

    const char * data() const
    {
        return buffer_->ptr() + sizeof(size_t);
    }

    size_t size() const
    {
        return *size_ptr();
    }

    size_t capacity() const
    {
        return buffer_->buffer_size() - sizeof(size_t);
    }

    void resize(size_t value)
    {
        BOOST_ASSERT(value <= capacity());
        *size_ptr() = value;
    }

    boost::asio::const_buffer make(size_t skip) const;
    
    void swap(nexus::Buffer & rhs)
    {
        buffer_.swap(rhs.buffer_);
    }

    typedef mstd::pbuffer Buffer::*unspecified_bool_type;

    operator unspecified_bool_type() const
    {
        return buffer_ ? &Buffer::buffer_ : 0;
    }

    static Buffer blank();
private:
    size_t * size_ptr() const
    {
        return mstd::pointer_cast<size_t*>(buffer_->ptr());
    }

    mstd::pbuffer buffer_;
};

inline void swap(nexus::Buffer & lhs, nexus::Buffer & rhs)
{
    lhs.swap(rhs);
}

}
