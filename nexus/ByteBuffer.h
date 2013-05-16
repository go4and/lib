#pragma once

namespace nexus {

#ifdef BOOST_WINDOWS
typedef WSABUF BufferDescriptor;
inline BufferDescriptor makeBufferDescriptor(void * data, size_t len)
{
    BufferDescriptor result = { static_cast<ULONG>(len), static_cast<CHAR*>(data) };
    return result;
}
#else
#endif

class ByteBuffer {
public:
    ByteBuffer(size_t initial);

    inline size_t trim(size_t value)
    {
        return value >= data_.size() ? value - data_.size() : value;
    }

    inline bool empty() const
    {
        return wpos_ == rpos_;
    }

    inline size_t ready() const
    {
        return rpos_ <= wpos_ ? (wpos_ - rpos_) : (data_.size() - rpos_ + wpos_);
    }

    inline void ready(size_t transferred)
    {
        wpos_ = trim(wpos_ + transferred);
    }

    inline void consume(size_t transferred)
    {
        rpos_ = trim(rpos_ + transferred);
    }

    void append(const char * data, size_t len);
    void ensure(size_t space);
    inline size_t prepareRecv(BufferDescriptor * bufs) { return prepare(bufs, wpos_, (rpos_ ? rpos_ : data_.size()) - 1); }
    size_t recv(BufferDescriptor * bufs, size_t count);
    size_t read(char * out, size_t limit) { BufferDescriptor bufs = makeBufferDescriptor(out, limit); size_t result = recv(&bufs, 1); return result; }
    size_t read(unsigned char * out, size_t limit) { return read(mstd::pointer_cast<char*>(out), limit); }
    inline size_t prepareSend(BufferDescriptor * bufs) { return prepare(bufs, rpos_, wpos_); }
private:
    size_t prepare(BufferDescriptor * bufs, size_t start, size_t stop);

    size_t wpos_;
    size_t rpos_;
    std::vector<char> data_;
};

}
