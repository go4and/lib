#pragma once

namespace nexus {

template<class Buffer>
class GenericBuffers;

template<class Buffer>
class GenericBuffersRef {
public:
    explicit GenericBuffersRef(const GenericBuffers<Buffer> & buffers)
        : buffers_(&buffers)
    {
    }

    class const_iterator {
    public:
        const_iterator(const typename boost::circular_buffer<Buffer>::const_iterator & impl, size_t skip)
            : impl_(impl), skip_(skip)
        {
        }
        
        void operator++()
        {
            ++impl_;
            skip_ = 0;
        }
        
        bool operator!=(const const_iterator & rhs) const
        {
            return impl_ != rhs.impl_;
        }

        boost::asio::const_buffer operator*() const
        {
            return boost::asio::const_buffer(impl_->data() + skip_, impl_->size() - skip_);
        }
    private:
        typename boost::circular_buffer<Buffer>::const_iterator impl_;
        size_t skip_;
    };

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
        while(transferred != 0)
        {
            Buffer & buf = pending_.front();
            if(buf.size() - skip_ <= transferred)
            {
                transferred -= buf.size() - skip_;
                skip_ = 0;
                pending_.pop_front();
            } else {
                skip_ += transferred;
                transferred = 0;
            }
        }
    }
private:
    boost::circular_buffer<Buffer> pending_;
    size_t skip_;
    
    friend class GenericBuffersRef<Buffer>;
};

}
