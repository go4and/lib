/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Buffers.h"

MLOG_DECLARE_LOGGER(buffers);

namespace nexus {

BuffersRef::BuffersRef(const Buffers & source)
    : skip_(source.skip_)
{
    std::deque<Buffer>::const_iterator i = source.buffers_.begin();
    size_ = std::min<size_t>(source.buffers_.size(), Value::static_size);
    Value::iterator o = value_.begin(), stop = o + size_;
    for(; o != stop; ++i, ++o)
        *o = *i;
}

Buffers::Buffers()
    : skip_(0), total_(0) {}

void Buffers::erase(size_t len)
{
    if(!len)
        return;

    BOOST_ASSERT(len <= total_);

    total_ -= len;

    std::deque<Buffer>::iterator begin = buffers_.begin(), i = begin;
    len += skip_;
    skip_ = 0;

    while(len)
    {
        size_t current = i->size();
        if(len < current) {
            skip_ = len;
            break;
        } else {
            len -= current;
            ++i;
        }
    }

    buffers_.erase(begin, i);
}

}
