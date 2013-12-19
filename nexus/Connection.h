/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef NEXUS_BUILDING

#include <vector>

#include <boost/asio/ip/tcp.hpp>

#include <boost/thread/mutex.hpp>

#include <boost/system/error_code.hpp>

#include <mstd/atomic.hpp>
#include <mstd/threads.hpp>

#include <mlog/Logging.h>

#endif

#include "Config.h"

#include "AsyncGuard.h"
#include "AsyncOperations.h"
#include "Buffers.h"
#include "Clock.h"
#include "Handler.h"
#include "PacketReader.h"

namespace nexus {

template<class Derived, class Guard, class Lazy, class AsyncData>
class Connection;

class ConnectionLock;

typedef int StopReason;
const int srNone = 0;
const int srRead = 1;
const int srWrite = 2;

class NEXUS_DECL ConnectionBase {
public:
    ConnectionBase(bool active, size_t readingBuffer, size_t threshold);
    ~ConnectionBase();

    bool active();
    int operations();

    size_t reads();
    size_t writes();
    int rpos();
    mstd::thread_id lastLocker();

    void stopReading(StopReason reason, const boost::system::error_code & ec = boost::system::error_code());

    int failedOperation() const
    {
        return stopReason_;
    }
    
    const boost::system::error_code & errorCode() const
    {
        return ec_;
    }

    virtual size_t sendQueueSize() = 0;

    Milliseconds lastRead() const
    {
        return lastRead_;
    }

    Milliseconds lastWrite() const
    {
        return lastWrite_;
    }

    void updateLastRead()
    {
        lastRead_ = Clock::milliseconds();
    }
    
    void updateLastWrite()
    {
        lastWrite_ = Clock::milliseconds();
    }

    static size_t activeConnections();
    static size_t allocatedConnections();
protected:
    bool activate();
    bool prepare();
    bool reading();
    void stopReason(StopReason reason, const boost::system::error_code & ec);
private:
    static mlog::Logger & getLogger();
    void commitWrite(size_t len, ConnectionLock & lock);

    AsyncOperations asyncOperations_;
    boost::mutex mutex_;
    Buffers pending_;
    std::vector<char> rbuffer_;
    size_t rpos_;
    size_t threshold_;
    mstd::atomic<size_t> reads_;
    mstd::atomic<size_t> writes_;
    mstd::atomic<bool> reading_;
    mstd::atomic<mstd::thread_id> lastLocker_;
    mstd::atomic<StopReason> stopReason_;
    boost::system::error_code ec_;
    mstd::atomic<Milliseconds> lastRead_;
    mstd::atomic<Milliseconds> lastWrite_;

    template<class, class, class, class>
    friend class Connection;
    
    friend class ConnectionLock;
};

class NEXUS_DECL ConnectionLock : public boost::noncopyable {
public:
    explicit ConnectionLock(ConnectionBase * conn)
        : m_(conn->mutex_)
    {
        m_.lock();
        conn->lastLocker_ = mstd::this_thread_id();
    }

    ~ConnectionLock()
    {
        m_.unlock();
    }
private:
    boost::mutex & m_;
};

class NEXUS_DECL NoGuard {
public:
    template<class T>
    const T & wrap(const T & t)
    {
        return t;
    }
};

class NEXUS_DECL NoLazyBuffer {
public:
    bool feed(const char * input, size_t len)
    {
        return false;
    }

    bool empty() const
    {
        return true;
    }

    Buffer commit()
    {
        BOOST_ASSERT(false);
        return Buffer::blank();
    }
};

template<size_t size>
class LazyBuffer {
public:
    LazyBuffer()
        : pos_(0) {}

    bool feed(const char * input, size_t len)
    {
        if(buffer_.size() - pos_ >= len)
        {
            memcpy(&buffer_[0] + pos_, input, len);
            pos_ += len;
            return true;
        } else
            return false;
    }

    bool empty() const
    {
        return pos_ == 0;
    }

    Buffer commit()
    {
        size_t pos = pos_;
        pos_ = 0;
        return Buffer(&buffer_[0], pos);
    }
private:
    boost::array<char, size> buffer_;
    size_t pos_;
};

struct NEXUS_DECL NoAsyncData {
    static NoAsyncData null() { return NoAsyncData(); }
};

template<class Derived, class Guard = NoGuard, class Lazy = NoLazyBuffer, class AD = NoAsyncData>
class Connection : public ConnectionBase {
public:
    typedef AD AsyncData;
    typedef nexus::Connection<Derived, Guard> base_type;

    Connection(bool active, size_t readingBuffer, size_t threshold = 0)
        : ConnectionBase(active, readingBuffer, threshold) {}

    template<class T>
    Connection(bool active, size_t readingBuffer, size_t threshold = 0, const T & t = T())
        : ConnectionBase(active, readingBuffer, threshold), guard_(t) {}

    size_t sendQueueSize()
    {
        ConnectionLock lock(this);
        return pending_.total();
    }

    void send(const Buffer & buffer)
    {
        if(asyncOperations_.active())
        {
            ConnectionLock lock(this);

            bool wasEmpty = pending_.empty();
            commitLazy(lock);

            pending_.push_back(buffer);

            if(wasEmpty)
                asyncWrite(lock);
        }
    }
    
    void send(const std::vector<Buffer> & buffers)
    {
        if(asyncOperations_.active())
        {
            ConnectionLock lock(this);

            bool wasEmpty = pending_.empty();
            commitLazy(lock);

            pending_.add(buffers);

            if(wasEmpty)
                asyncWrite(lock);
        }
    }

    void send(const char * data, size_t len)
    {
        if(asyncOperations_.active())
        {
            ConnectionLock lock(this);

            if(pending_.empty())
            {
                pending_.push_back(Buffer(data, len));
                asyncWrite(lock);
            } else if(!lazy_.feed(data, len))
            {
                commitLazy(lock);
                if(!lazy_.feed(data, len))
                    pending_.push_back(Buffer(data, len));
            }
        }
    }

    void post(const void * buffer, size_t size)
    {
        if(asyncOperations_.prepare())
            derived().stream().io_service().post(Send<Buffer>(this, Buffer(static_cast<const char*>(buffer), size)));
    }

    void post(const nexus::Buffer & buffer)
    {
        if(asyncOperations_.prepare())
            derived().stream().get_io_service().post(Send<Buffer>(this, buffer));
    }

    template<class C>
    void postBuffers(const C & c)
    {
        if(asyncOperations_.prepare())
            derived().stream().get_io_service().post(Send<C>(this, c));
    }
private:
    class AsyncHelper;
protected:
    bool stop(StopReason reason, const boost::system::error_code & ec = boost::system::error_code())
    {
        stopReason(reason, ec);
        if(asyncOperations_.shutdown())
        {
            derived().shutdown();
            return true;
        } else
            return false;
    }

    void start()
    {
        asyncRead();
    }
    
    Guard & guard()
    {
        return guard_;
    }
    
    std::pair<const char *, const char *> readyData()
    {
        return std::make_pair(&rbuffer_[0], &rbuffer_[0] + rpos_);
    }
    
    typedef nexus::AsyncGuard<AsyncHelper> AsyncGuard;
private:
    class AsyncHelper {
    public:
        typedef typename Connection::AsyncData AsyncData;

        explicit AsyncHelper(Connection * conn, AsyncData data)
            : conn_(conn), data_(data) {}

        AsyncOperations & asyncOperations()
        {
            return conn_->asyncOperations_;
        }

        void shutdown()
        {
            conn_->doShutdown();
        }

        void finish()
        {
            conn_->doFinish(data_);
        }
    private:
        Connection * conn_;
        AsyncData data_;
    };
    
    Derived & derived()
    {
        return *static_cast<Derived*>(this);
    }

    void asyncRead()
    {
        if(reading() && asyncOperations_.prepare())
        {
            ++reads_;

            size_t bsize = rbuffer_.size();
            if(threshold_ && (bsize - rpos_) * threshold_ < bsize)
            { 
                bsize *= 2;
                while((bsize - rpos_) * threshold_ < bsize)
                    bsize *= 2;
                rbuffer_.resize(bsize);
                rbuffer_.resize(bsize = rbuffer_.capacity());
            }

            size_t size = bsize - rpos_;

            ConnectionLock lock(this);

            derived().stream().async_read_some(boost::asio::buffer(&rbuffer_[rpos_], size),
                                               guard_.wrap(bindRead(baseAsyncData<AsyncData>())));
        }
    }

    void commitLazy(ConnectionLock &)
    {
        if(!lazy_.empty())
            pending_.push_back(lazy_.commit());
    }

    void handleWrite(const boost::system::error_code & ec, size_t len, AsyncData data)
    {
        MLOG_FMESSAGE(Debug, "handleWrite(" << ec << ", " << len << ")");

        --writes_;

        AsyncGuard guard(this, data);

        if(!ec)
        {
            updateLastWrite();
        
            ConnectionLock lock(this);

            commitWrite(len, lock);
            if(pending_.mayAdd())
                commitLazy(lock);
            asyncWrite(lock);
        } else {
            MLOG_FMESSAGE(Notice, "handleWrite(" << ec << ", " << ec.message() << ")");

            guard.failed();
            stopReason(srWrite, ec);
        }
    }

    void asyncWrite(ConnectionLock & lock)
    {
        if(!pending_.empty())
        {
            if(asyncOperations_.prepare())
            {
                ++writes_;

                derived().stream().async_write_some(pending_.ref(), guard_.wrap(bindWrite(baseAsyncData<AsyncData>())));
            } else
                pending_.clear();
        } else if(!reading())
            derived().shutdown();
    }

    void handleRead(boost::system::error_code ec, size_t len, AsyncData data)
    {
        --reads_;

        AsyncGuard guard(this, data);

        if(!ec)
        {
            rpos_ += len;
            updateLastRead();

            PacketReader reader(rbuffer_, rpos_);
            derived().processPackets(reader);
            memmove(&rbuffer_[0], reader.raw(), reader.left());
            rpos_ = reader.left();

            asyncRead();
        } else {
            MLOG_FMESSAGE(Notice, "handleRead(" << ec << ", " << ec.message() << ")");

            guard.failed();
            stopReason(srRead, ec);
        }
    }

    void doFinish(AsyncData data)
    {
        {
            ConnectionLock lock(this);

            pending_.clear();
        }
        rpos_ = 0;
        
        invokeFinish(data);
    }

    void doShutdown()
    {
        derived().shutdown();
    }

    template<class Data>
    typename boost::enable_if<boost::is_same<Data, NoAsyncData>, void>::type
    invokeFinish(Data data)
    {
        derived().finish();
    }

    template<class Data>
    typename boost::disable_if<boost::is_same<Data, NoAsyncData>, void>::type
    invokeFinish(Data data)
    {
        derived().finish(data);
    }

    template<class Data>
    typename boost::enable_if<boost::is_same<Data, NoAsyncData>, AsyncData>::type
    baseAsyncData()
    {
        return AsyncData::null();
    }

    template<class Data>
    typename boost::disable_if<boost::is_same<Data, NoAsyncData>, AsyncData>::type
    baseAsyncData()
    {
        return derived().asyncData();
    }

    template<class C>
    class Send {
    public:
        Send(Connection * conn, const C & c)
            : conn_(conn), c_(c)
        {
        }

        void operator()()
        {
            AsyncGuard guard(conn_);
            conn_->send(c_);
        }
    private:
        Connection * conn_;
        C c_;
    };

    NEXUS_DECLARE_HANDLER(Read, Connection, true);
    NEXUS_DECLARE_HANDLER(Write, Connection, true);

    Guard guard_;
    Lazy lazy_;

    friend class AsyncHelper;
    friend class SendPBuffer;
};

}
