#include "pch.h"

#include "Manager.h"
#include "Output.h"

#if !MLOG_NO_LOGGING

namespace mlog {

namespace detail {

const size_t bufferSize = 0x100;

struct Buffer {
    boost::array<char, bufferSize> data;
    Buffer * next;

    Buffer()
        : next(0) {}
};

class StreamBuf : public std::streambuf {
public:
    StreamBuf()
        : cur_(&buffer_)
    {
        char * begin = &cur_->data[0];
        std::streambuf::setp(begin, begin + bufferSize);
    }

    ~StreamBuf()
    {
        reset();
    }

	int_type overflow(int_type c)
	{
        cur_->next = new Buffer;
        cur_ = cur_->next;
        cur_->data[0] = c;
        char * begin = &cur_->data[0];
        std::streambuf::setp(begin + 1, begin + bufferSize);
        return c;
    }

    std::streamsize xsputn(const char_type * s, std::streamsize n)
    {
        const char_type * end = s + n;
        size_t c = std::min<size_t>(end - s, epptr() - pptr());
        memcpy(pptr(), s, c);
        pbump(static_cast<int>(c));
        s += c;
        if(s != end)
        {
            size_t c;
            cur_->next = new Buffer;
            cur_ = cur_->next;
            for(;;)
            {
                c = std::min<size_t>(end - s, bufferSize);
                memcpy(&cur_->data[0], s, c);
                s += c;
                if(s == end)
                    break;
                cur_->next = new Buffer;
                cur_ = cur_->next;
            }
            char * begin = &cur_->data[0];
            std::streambuf::setp(begin + c, begin + bufferSize);
        }
        return n;
    }

    void reset()
    {
        cur_ = &buffer_;
        Buffer * c = buffer_.next;
        while(c)
        {
            Buffer * n = c->next;
            delete c;
            c = n;
        }
        buffer_.next = 0;

        char * begin = &cur_->data[0];
        std::streambuf::setp(begin, begin + bufferSize);
    }

    mlog::Buffer buffer(uint32_t group, mlog::LogLevel level)
    {
        size_t size = 0;
        for(Buffer * c = buffer_.next; c; c = c->next)
            ++size;
        size_t pos = pptr() - &cur_->data[0];
        size = size * bufferSize + pos;
        size_t resultSize = size + sizeof(size_t) + sizeof(group) + sizeof(level) + 1;
#if MLOG_USE_BUFFERS
        mstd::pbuffer result = mstd::buffers::instance().take(resultSize);
#else
        mstd::rc_buffer result(resultSize);
#endif
        char * p = bufferData(result);
        *mstd::pointer_cast<uint32_t*>(p) = group;
        p += sizeof(group);
        *mstd::pointer_cast<LogLevel*>(p) = level;
        p += sizeof(level);
        *mstd::pointer_cast<size_t*>(p) = size;
        p += sizeof(size);
        for(Buffer * c = &buffer_; ; c = c->next)
        {
            if(!c->next)
            {
                memcpy(p, &c->data[0], pos);                
                p += pos;
                break;
            } else {
                memcpy(p, &c->data[0], bufferSize);
                p += bufferSize;
            }
        }
        *p = 0;
        return result;
    }
private:
    Buffer buffer_;
    Buffer * cur_;
};

class StreamBase {
protected:
    StreamBuf buf_;
};

class Stream : private StreamBase, public std::ostream {
public:
    Stream()
        : std::ostream(&buf_) {}
};

class OStreamPool::Impl {
public:
    boost::ptr_vector<Stream> streams;
    boost::mutex mutex;
};

OStreamPool::OStreamPool()
    : impl_(new Impl)
{
}

OStreamPool::~OStreamPool()
{
}

std::ostream * OStreamPool::take()
{
    boost::mutex::scoped_lock lock(impl_->mutex);
    if(impl_->streams.empty())
        return new Stream;
    else
        return impl_->streams.pop_back().release();
}

void OStreamPool::release(std::ostream * stream)
{
    static_cast<StreamBuf*>(stream->rdbuf())->reset();
    boost::lock_guard<boost::mutex> lock(impl_->mutex);
    impl_->streams.push_back(static_cast<Stream*>(stream));
}

}

void Output::send(uint32_t group, LogLevel level, const char * logger)
{
#if !defined(BOOST_WINDOWS) && !defined(__APPLE__)
    (*out_) << "\033[0m";
#endif
    (*out_) << '\n';
    Manager::instance().output(logger, static_cast<detail::StreamBuf*>(out_->rdbuf())->buffer(group, level));
}

}

#endif
