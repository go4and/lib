/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "pch.h"

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

    setBufferDescriptorData(bufs[0], &data_[0] + start);
    if(stop > start)
    {
        setBufferDescriptorSize(bufs[0], stop - start);
        return 1;
    } else {
        setBufferDescriptorSize(bufs[0], data_.size() - start);
        if(stop)
        {
            bufs[1] = makeBufferDescriptor(&data_[0], stop);
            return 2;
        } else
            return 1;
    }
}

size_t ByteBuffer::recv(BufferDescriptor * bufs, size_t count)
{
    MLOG_DEBUG("recv(" << count << "), rpos: " << rpos_ << ", wpos: " << wpos_);

    size_t result = 0;
    size_t rpos = rpos_;
    size_t ready = this->ready();
    for(size_t i = 0; ready && i != count; ++i)
    {
        size_t len = std::min(ready, bufferDescriptorSize(bufs[i]));
        ready -= len;
        result += len;
        if(rpos + len > data_.size())
        {
            size_t tail = data_.size() - rpos_;
            char * out = static_cast<char*>(bufferDescriptorData(bufs[i]));
            memcpy(out, &data_[0] + rpos_, tail);
            memcpy(out + tail, &data_[0], len - tail);
        } else {
            memcpy(bufferDescriptorData(bufs[i]), &data_[0] + rpos_, len);
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
