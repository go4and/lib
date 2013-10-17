/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace nexus {

template<class Buffer, class Op>
inline typename Op::result_type operation(std::vector<char> & buffer, size_t begin, size_t end, const Op & op, size_t & len)
{
    BOOST_ASSERT(begin != end);

    size_t bufferSize = buffer.size();
    if(end > begin)
    {
        len = end - begin;
        return op(boost::asio::buffer(&buffer[begin], end - begin));
    } else if(end)
    {
        len = (bufferSize - begin) + end;
        boost::array<Buffer, 0x02> buffers = {
            Buffer(&buffer[begin], bufferSize - begin),
            Buffer(&buffer[0], end)
        };
        return op(buffers);
    } else {
        len = bufferSize - begin;
        return op(boost::asio::buffer(&buffer[begin], bufferSize - begin));
    }
}

template<class Position = volatile size_t>
class RingBuffer {
public:
    typedef Position position_type;

    RingBuffer(size_t size)
        : bufferSize_(size), buffer_(size)
    {
        buffer_.resize(buffer_.capacity());
        bufferSize_ = buffer_.size();
    }

    void reset()
    {
        writePos_ = 0;
        readPos_ = 0;
    }

    inline size_t trim(size_t idx) const
    {
        return idx >= bufferSize_ ? idx - bufferSize_ : idx;
    }

    inline size_t writePosition() const { return writePos_; }
    inline size_t readPosition() const { return readPos_; }

    inline bool full() const
    {
        return trim(writePos_ + 1) == readPos_;
    }

    inline bool empty() const
    {
        return writePos_ == readPos_;
    }

    inline void moveRead(size_t transferred)
    {
        readPos_ = trim(readPos_ + transferred);
    }

    inline void moveWrite(size_t transferred)
    {
        writePos_ = trim(writePos_ + transferred);
    }
    
    template<class Op>
    inline typename Op::result_type read(const Op & op, size_t & len)
    {
        return operation<boost::asio::const_buffer>(buffer_, readPos_, writePos_, op, len);
    }
    
    template<class Op>
    inline typename Op::result_type write(const Op & op, size_t & len)
    {
        return operation<boost::asio::mutable_buffer>(buffer_, writePos_, trim(readPos_ + bufferSize_ - 1), op, len);
    }
private:
    std::vector<char> buffer_;
    size_t bufferSize_;
    position_type writePos_;
    position_type readPos_;
};

template<class Handler>
class AsyncReadSome {
public:
    typedef void result_type;

    AsyncReadSome(boost::asio::ip::tcp::socket & socket, const Handler & handler)
        : socket_(socket), handler_(handler)
    {
    }

    template<class Buffers>
    void operator()(const Buffers & buffers) const
    {
        socket_.async_read_some(buffers, handler_);
    }
private:
    boost::asio::ip::tcp::socket & socket_;
    const Handler & handler_;
};

template<class Handler>
AsyncReadSome<Handler> asyncReadSome(boost::asio::ip::tcp::socket & socket, const Handler & handler)
{
    return AsyncReadSome<Handler>(socket, handler);
}

class ReadSome {
public:
    typedef size_t result_type;

    ReadSome(boost::asio::ip::tcp::socket & socket, boost::system::error_code & ec)
        : socket_(socket), ec_(ec)
    {
    }

    template<class Buffers>
    result_type operator()(const Buffers & buffers) const
    {
        return socket_.receive(buffers, 0, ec_);
    }
private:
    boost::asio::ip::tcp::socket & socket_;
    boost::system::error_code & ec_;
};

inline ReadSome readSome(boost::asio::ip::tcp::socket & socket, boost::system::error_code & ec)
{
    return ReadSome(socket, ec);
}

template<class Handler>
class AsyncWriteSome {
public:
    typedef void result_type;

    AsyncWriteSome(boost::asio::ip::tcp::socket & socket, const Handler & handler)
        : socket_(socket), handler_(handler)
    {
    }

    template<class Buffers>
    void operator()(const Buffers & buffers) const
    {
        socket_.async_write_some(buffers, handler_);
    }
private:
    boost::asio::ip::tcp::socket & socket_;
    const Handler & handler_;
};

template<class Handler>
AsyncWriteSome<Handler> asyncWriteSome(boost::asio::ip::tcp::socket & socket, const Handler & handler)
{
    return AsyncWriteSome<Handler>(socket, handler);
}

class WriteSome {
public:
    typedef size_t result_type;

    WriteSome(boost::asio::ip::tcp::socket & socket, boost::system::error_code & ec)
        : socket_(socket), ec_(ec)
    {
    }

    template<class Buffers>
    result_type operator()(const Buffers & buffers) const
    {
        return socket_.write_some(buffers, ec_);
    }
private:
    boost::asio::ip::tcp::socket & socket_;
    boost::system::error_code & ec_;
};

inline WriteSome writeSome(boost::asio::ip::tcp::socket & socket, boost::system::error_code & ec)
{
    return WriteSome(socket, ec);
}

}
