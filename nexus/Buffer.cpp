/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Buffer.h"

namespace nexus {

namespace {

mstd::buffers & bufs = mstd::buffers::instance();

}

Buffer::Buffer(size_t len)
    : buffer_(bufs.take(len + sizeof(size_t)))
{
    *size_ptr() = len;
}

Buffer::Buffer(const char * source, size_t len)
    : buffer_(bufs.take(len + sizeof(size_t)))
{
    *size_ptr() = len;
    memcpy(data(), source, len);
}

Buffer::Buffer(const char * begin, const char * end)
    : buffer_(bufs.take(end - begin + sizeof(size_t)))
{
    *size_ptr() = end - begin;
    memcpy(data(), begin, end - begin);
}

boost::asio::const_buffer Buffer::make(size_t skip) const
{
    return boost::asio::const_buffer(data() + skip, size() - skip);
}

class BlankBuffer : public mstd::singleton<BlankBuffer> {
public:
    Buffer value() const
    {
        return value_;
    }
private:
    BlankBuffer()
        : value_(static_cast<const char*>(0), static_cast<size_t>(0)) {}

    Buffer value_;

    friend class mstd::singleton<BlankBuffer>;
};

Buffer Buffer::blank()
{
    return BlankBuffer::instance().value();
}

}
