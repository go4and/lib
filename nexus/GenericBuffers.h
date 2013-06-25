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
#include <boost/circular_buffer.hpp>
#endif

namespace nexus {

template<class Buffer>
class GenericBuffers;

template<class T>
const void * get_buffer_data(const T & t)
{
    return t.data();
}

template<class T>
size_t get_buffer_size(const T & t)
{
    return t.size();
}

class GenericBufferTraits {
public:
    template<class T>
    static auto data(const T & t) -> decltype(get_buffer_data(t))
    {
        return get_buffer_data(t);
    }

    template<class T>
    static size_t size(const T & t)
    {
        return get_buffer_size(t);
    }

    template<class T>
    static auto data(T & t) -> decltype(get_buffer_data(t))
    {
        return get_buffer_data(t);
    }

    template<class T>
    static size_t size(T & t)
    {
        return get_buffer_size(t);
    }
};

template<class Impl, bool mutable_, class Traits = GenericBufferTraits>
class GenericBuffersIterator {
public:
    typedef typename boost::mpl::if_c<!mutable_, boost::asio::const_buffer, boost::asio::mutable_buffer>::type value_type;
    typedef typename boost::mpl::if_c<!mutable_, const char *, char *>::type data_type;

    GenericBuffersIterator(const Impl & impl, size_t skip)
        : impl_(impl), skip_(skip)
    {
    }
    
    void operator++()
    {
        ++impl_;
        skip_ = 0;
    }
    
    bool operator!=(const GenericBuffersIterator & rhs) const
    {
        return impl_ != rhs.impl_;
    }

    value_type operator*() const
    {
        data_type data = static_cast<data_type>(Traits::data(*impl_));
        return value_type(data + skip_, Traits::size(*impl_) - skip_);
    }
private:
    Impl impl_;
    size_t skip_;
};

template<class Buffer>
class GenericBuffersRef {
public:
    typedef GenericBuffersIterator<typename boost::circular_buffer<Buffer>::const_iterator, false> const_iterator;

    explicit GenericBuffersRef(const GenericBuffers<Buffer> & buffers)
        : buffers_(&buffers)
    {
    }

    const_iterator begin() const
    {
        return const_iterator(buffers_->pending_.begin(), buffers_->skip_);
    }

    const_iterator end() const
    {
        return const_iterator(buffers_->pending_.end(), 0);
    }
private:
    const GenericBuffers<Buffer> * buffers_;
};

template<class Pending>
void consume(size_t transferred, Pending & pending, size_t & skip)
{
    if(transferred && !pending.empty())
    {
        {
            size_t size = get_buffer_size(pending.front());
            size_t left = size - skip;
            if(left <= transferred)
            {
                transferred -= left;
                skip = 0;
                pending.pop_front();
            } else {
                skip += transferred;
                return;
            }
        }
        while(transferred != 0)
        {
            size_t size = get_buffer_size(pending.front());
            if(size <= transferred)
            {
                transferred -= size;
                pending.pop_front();
            } else {
                skip = transferred;
                break;
            }
        }
    }
}

template<class Buffer>
class GenericBuffers : public boost::noncopyable {
public:
    GenericBuffers()
        : pending_(8), skip_(0)
    {
    }

    GenericBuffersRef<Buffer> ref() const
    {
        return GenericBuffersRef<Buffer>(*this);
    }

    bool empty() const
    {
        return pending_.empty();
    }
    
    void push_back(const Buffer & value)
    {
        if(!pending_.reserve())
            pending_.set_capacity(pending_.capacity() * 2);
        pending_.push_back(value);
    }
    
    void consume(size_t transferred)
    {
        nexus::consume(transferred, pending_, skip_);
    }
private:
    boost::circular_buffer<Buffer> pending_;
    size_t skip_;
    
    friend class GenericBuffersRef<Buffer>;
};

}
