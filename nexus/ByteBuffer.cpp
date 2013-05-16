#include "pch.h"

#include "ByteBuffer.h"

MLOG_DECLARE_LOGGER(nexus_byte_buffer);

namespace nexus {

ByteBuffer::ByteBuffer(size_t initial)
    : wpos_(0), rpos_(0), data_(initial)
{
    data_.resize(data_.capacity());
}

void ByteBuffer::ensure(size_t space)
{
    size_t rpos = rpos_;
    if(rpos <= wpos_)
        rpos += data_.size();
    if(rpos - wpos_ - 1 < space)
    {
        MLOG_DEBUG("ensure(" << space << "), rpos: " << rpos_ << ", wpos: " << wpos_ << ", size: " << data_.size());

        std::vector<char> newBuffer(data_.size() + space);
        newBuffer.resize(newBuffer.capacity());
        if(rpos_ < wpos_)
        {
            memcpy(&newBuffer[0], &data_[0] + rpos_, wpos_ - rpos_);
            wpos_ -= rpos_;
        } else if(wpos_ < rpos_) {
            size_t tail = data_.size() - rpos_;
            memcpy(&newBuffer[0], &data_[0] + rpos_, tail);
            memcpy(&newBuffer[0] + tail, &data_[0], wpos_);
            wpos_ += tail;
        } else
            wpos_ = 0;
        rpos_ = 0;
        data_.swap(newBuffer);
    }
}

size_t ByteBuffer::prepare(BufferDescriptor * bufs, size_t start, size_t stop)
{
    MLOG_DEBUG("prepare(" << start << ", " << stop << "), size: " << data_.size());

    bufs[0].buf = &data_[0] + start;
    if(stop > start)
    {
        bufs[0].len = static_cast<ULONG>(stop - start);
        return 1;
    } else {
        bufs[0].len = static_cast<ULONG>(data_.size() - start);
        if(stop)
        {
            bufs[1].buf = &data_[0];
            bufs[1].len = static_cast<ULONG>(stop);
            return 2;
        } else
            return 1;
    }
}

size_t ByteBuffer::recv(BufferDescriptor * bufs, size_t count)
{
    MLOG_DEBUG("recv(" << count << "), rpos: " << rpos_ << ", wpos: " << wpos_);

    DWORD result = 0;
    size_t rpos = rpos_;
    size_t ready = this->ready();
    for(size_t i = 0; ready && i != count; ++i)
    {
        size_t len = std::min<size_t>(ready, bufs[i].len);
        ready -= len;
        result += static_cast<DWORD>(len);
        if(rpos + len > data_.size())
        {
            size_t tail = data_.size() - rpos_;
            memcpy(bufs[i].buf, &data_[0] + rpos_, tail);
            memcpy(bufs[i].buf + tail, &data_[0], len - tail);
        } else {
            memcpy(bufs[i].buf, &data_[0] + rpos_, len);
        }
        rpos = trim(rpos + len);
    }
    return result;
}

void ByteBuffer::append(const char * data, size_t len)
{
    MLOG_DEBUG("append(" << len << ")");

    ensure(len);
    size_t start = wpos_;
    size_t stop = (rpos_ ? rpos_ : data_.size()) - 1;
    if(stop > start)
    {
        memcpy(&data_[0] + start, data, len);
    } else {
        size_t left = data_.size() - start;
        if(left >= len)
            memcpy(&data_[0] + start, data, len);
        else {
            memcpy(&data_[0] + start, data, left);
            memcpy(&data_[0], data + left, len - left);
        }
    }
    wpos_ = trim(wpos_ + len);
}

}
