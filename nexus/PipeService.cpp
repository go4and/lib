/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#ifdef BOOST_WINDOWS

#include "ChunkedBuffer.h"
#include "PacketReader.h"
#include "PipeService.h"

MLOG_DECLARE_LOGGER(nexus_pipe_service);

namespace nexus {

namespace {

class IoOperation : private OVERLAPPED {
public:
    virtual void complete(DWORD bytesTransferred, int err) = 0;
    virtual ~IoOperation() {}

    OVERLAPPED * prepare()
    {
        OVERLAPPED * over = this;
        memset(over, 0, sizeof(*over));
        return over;
    }
};

template<class F>
class ActionOperation : public IoOperation {
public:
    explicit ActionOperation(const F & f)
        : f_(f)
    {
    }

    void complete(DWORD bytesTransferred, int err)
    {
        f_();
    }
private:
    F f_;
};

template<class F>
IoOperation * actionOperation(const F & f)
{
    return new ActionOperation<F>(f);
}

class PipeConnection;

class ConnectionOperation : public IoOperation {
public:
    explicit ConnectionOperation(PipeConnection & connection)
        : connection_(connection)
    {
    }
protected:
    PipeConnection & connection_;
};

class ConnectOperation : public ConnectionOperation {
public:
    ConnectOperation(PipeConnection & connection)
        : ConnectionOperation(connection)
    {
    }

    void complete(DWORD bytesTransferred, int err);
};

class ReceiveOperation : public ConnectionOperation {
public:
    ReceiveOperation(PipeConnection & connection)
        : ConnectionOperation(connection)
    {
    }

    void complete(DWORD bytesTransferred, int err);
};

class SendOperation : public ConnectionOperation {
public:
    SendOperation(PipeConnection & connection)
        : ConnectionOperation(connection)
    {
    }

    void complete(DWORD bytesTransferred, int err);
};

class PipeConnectionContext {
public:
    virtual HANDLE iocp() = 0;
    virtual void connectDone() = 0;
    virtual uint32_t registerConnection(PipeConnection * connection) = 0;
    virtual void releaseConnection(PipeConnection * connection) = 0;
};

class PipeConnection : boost::noncopyable {
public:
    PipeConnection(PipeConnectionContext & context, const PipeService::Listener & listener, HANDLE handle)
        : context_(context), listener_(listener), id_(0), handle_(handle), activeOperations_(0),
          recvPos_(0), connectOp_(*this), receiveOp_(*this), sendOp_(*this)
    {
        if(CreateIoCompletionPort(handle, context_.iocp(), 0, 0) == 0)
        {
            int err = GetLastError();
            MLOG_ERROR("register failed: " << err);
        }
    }

    void connect()
    {
        bool ok = ConnectNamedPipe(handle_, connectOp_.prepare()) != 0;
        int error = GetLastError();
        
        MLOG_DEBUG("connect result: " << ok << ", " << error);
        if(!ok)
        {
            if(error == ERROR_PIPE_CONNECTED)
                connectDone(0);
            else if(error != ERROR_IO_PENDING)
            {
                MLOG_ERROR("connect failed: " << error);
                connectDone(error);
            }
        } else
            connectDone(0);
    }

    uint32_t id() const
    {
        return id_;
    }

    ~PipeConnection()
    {
        reset();
    }

    void reset()
    {
        if(handle_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
        }
    }

    void send(const char * data, size_t len)
    {
        bool wasEmpty = sendBuffer_.empty();
        sendBuffer_.append(data, len);
        if(wasEmpty && len)
            startSend();
    }

    void connectDone(int error)
    {
        context_.connectDone();
        if(!error)
        {
            id_ = context_.registerConnection(this);
            int err = startReceive();
            if(!err)
            {
                ++activeOperations_;
                listener_(id_, -pcConnected, 0, 0);
            } else
                listener_(-err, -pcFailed, 0, 0);
        } else {
            listener_(-error, -pcFailed, 0, 0);
            context_.releaseConnection(this);
        }
    }

    void receiveDone(size_t len, int error)
    {
        if(!error && handle_ != INVALID_HANDLE_VALUE)
        {
            recvPos_ += len;
            processPackets();
            if(!startReceive())
               return; 
        }
        --activeOperations_;
        reset();
        checkFinished();
    }

    void sendDone(size_t len, int error)
    {
        --activeOperations_;
        if(!error)
        {
            sendBuffer_.consume(len);
            if(!sendBuffer_.empty())
                startSend();
        } else
            reset();
        checkFinished();
    }

    void disconnect()
    {
        reset();
    }
private:
    void processPackets()
    {
        MLOG_DEBUG("processPackets(" << mlog::dump(&recvBuffer_[0], recvPos_) << ")");

        PacketReader reader(&recvBuffer_[0], recvPos_);
        while(reader.left() >= 2)
        {
            reader.mark();
            PacketCode code = reader.read<PacketCode>();
            uint32_t len = reader.read<uint8_t>();
            if(len & 0x80)
            {
                if(reader.left() >= 1)
                {
                    len = (len & 0x7f) | (reader.read<uint8_t>() << 7);
                    if(len & 0x4000)
                    {
                        if(reader.left() >= 2)
                            len = (len & 0x3fff) | (reader.read<uint16_t>() << 14);
                    }
                }
            }
            if(len > reader.left())
            {
                reader.revert();
                break;
            }
            listener_(id_, code, reader.raw(), len);
            reader.skip(len);
        }
        memmove(&recvBuffer_[0], reader.raw(), reader.left());
        recvPos_ = reader.left();
    }

    void checkFinished()
    {
        if(activeOperations_ == 0)
        {
            MLOG_NOTICE("pipe finished: " << id_);
            listener_(id_, -pcDisconnected, 0, 0);
            context_.releaseConnection(this);
        }
    }

    void startSend()
    {
        if(handle_ != INVALID_HANDLE_VALUE)
        {
            auto p = sendBuffer_.readyChunk();
            bool res = WriteFile(handle_, p.first, p.second, 0, sendOp_.prepare()) != 0;
            int err = !res ? GetLastError() : 0;
            MLOG_DEBUG("send result: " << res << ", " << err);
            if(res || err == ERROR_IO_PENDING)
                ++activeOperations_;
            else {
                MLOG_ERROR("send failed: " << err);
                reset();
            }
        }
    }

    int startReceive()
    {
        if(recvBuffer_.size() - recvPos_ < 0x100)
        {
            recvBuffer_.resize(recvBuffer_.size() + 0x100);
            recvBuffer_.resize(recvBuffer_.capacity());
        }
        bool ok = ReadFile(handle_, &recvBuffer_[recvPos_], recvBuffer_.size() - recvPos_, 0, receiveOp_.prepare()) != 0; 
        int err = ok ? 0 : GetLastError();
        MLOG_DEBUG("recv result: " << ok << ", " << err);
        if(err == ERROR_IO_PENDING)
            err = 0;
        if(err)
            MLOG_ERROR("recv failed: " << err);
        return err;
    }

    PipeConnectionContext & context_;
    PipeService::Listener listener_;
    uint32_t id_;
    HANDLE handle_;
    int activeOperations_;

    std::vector<char> recvBuffer_;
    size_t recvPos_;

    ChunkedBuffer sendBuffer_;

    ConnectOperation connectOp_;
    ReceiveOperation receiveOp_;
    SendOperation sendOp_;
};

void ConnectOperation::complete(DWORD bytesTransferred, int err)
{
    MLOG_DEBUG("ConnectOperation::complete(" << bytesTransferred << ", " << err << ")");

    connection_.connectDone(err);
}

void ReceiveOperation::complete(DWORD bytesTransferred, int err)
{
    MLOG_DEBUG("ReceiveOperation::complete(" << bytesTransferred << ", " << err << ")");

    connection_.receiveDone(bytesTransferred, err);
}

void SendOperation::complete(DWORD bytesTransferred, int err)
{
    MLOG_DEBUG("SendOperation::complete(" << bytesTransferred << ", " << err << ")");

    connection_.sendDone(bytesTransferred, err);
}

struct TimerOperation {
    boost::posix_time::ptime time;

    explicit TimerOperation(const boost::posix_time::ptime & t)
        : time(t)
    {
        MLOG_DEBUG("TimerOperation(" << t << "), this = " << this);
    }

    virtual void run() = 0;
    virtual ~TimerOperation()
    {
        MLOG_DEBUG("~TimerOperation(), this = " << this);
    }
};

template<class F>
struct TimerOperationImpl : public TimerOperation {
    TimerOperationImpl(const boost::posix_time::ptime & time, const F & f)
        : TimerOperation(time), f_(f)
    {
    }

    virtual void run()
    {
        f_();
    }
private:
    F f_;
};

template<class F>
TimerOperation * timerOperation(const boost::posix_time::ptime & time, const F & f)
{
    return new TimerOperationImpl<F>(time, f);
}

template<class F>
TimerOperation * timerOperation(const boost::posix_time::time_duration & duration, const F & f)
{
    return new TimerOperationImpl<F>(boost::posix_time::microsec_clock::universal_time() + duration, f);
}

struct CompareTime {
    bool operator()(TimerOperation * lhs, TimerOperation * rhs) const
    {
        return lhs->time > rhs->time;
    }
};

mstd::rc_buffer pack(PacketCode code, const char * begin, size_t len)
{
    if(len < 0x80)
    {
        mstd::rc_buffer result(len + 2);
        unsigned char * out = result.udata();
        *out++ = code;
        *out++ = len;
        memcpy(out, begin, len);
        return result;
    } else if(len < 0x3fff) {
        mstd::rc_buffer result(len + 3);
        unsigned char * out = result.udata();
        *out++ = code;
        *out++ = (len & 0x7f) | 0x80;
        *out++ = len >> 7;
        memcpy(out, begin, len);
        return result;
    } else {
        mstd::rc_buffer result(len + 5);
        unsigned char * out = result.udata();
        *out++ = code;
        *out++ = static_cast<uint8_t>(len | 0x80);
        *out++ = static_cast<uint8_t>((len >> 7) | 0x80);
        *out++ = static_cast<uint8_t>(len >> 14);
        *out++ = static_cast<uint8_t>(len >> 22);
        memcpy(out, begin, len);
        return result;
    }
}

}

class PipeService::Impl : public PipeConnectionContext {
public:
    Impl()
        : iocp_(INVALID_HANDLE_VALUE)
    {
        connections_.erase(connections_.insert(0));
    }

    ~Impl()
    {
        if(thread_ != boost::thread())
        {
            thread_.interrupt();
            thread_.join();
        }

        if(iocp_ != INVALID_HANDLE_VALUE)
            CloseHandle(iocp_);
    }

    void listen(const Listener & listener, const std::wstring & name)
    {
        start();

        post(boost::bind(&Impl::doListen, this, listener, name));
    }

    void connect(const Listener & listener, const std::wstring & name, int retries)
    {
        start();

        post(boost::bind(&Impl::doConnect, this, listener, name, retries));
    }

    HANDLE iocp()
    {
        return iocp_;
    }

    void connectDone()
    {
        if(!listenName_.empty())
            post(boost::bind(&Impl::startListen, this));
    }

    uint32_t registerConnection(PipeConnection * conn)
    {
        return connections_.insert(conn).full();
    }

    void releaseConnection(PipeConnection * conn)
    {
        connections_.erase(conn->id());
        post(boost::bind(&Impl::deleteConnection, this, conn));
    }

    void send(int id, PacketCode code, const char * begin, size_t len)
    {
        MLOG_DEBUG("send(" << id << ", " << static_cast<int>(code) << ", " << mlog::dump(begin, len) << ")");

        post(boost::bind(&Impl::doSend, this, id, pack(code, begin, len)));
    }

    void disconnect(int id)
    {
        auto i = connections_.find(id);
        if(i != connections_.end())
            return (*i)->disconnect();
    }

    template<class F>
    void post(const F & f)
    {
        IoOperation * iop = actionOperation(f);
        PostQueuedCompletionStatus(iocp_, 0, 1, iop->prepare());
    }
private:
    void deleteConnection(PipeConnection * conn)
    {
        delete conn;
    }

    void start()
    {
        if(thread_ == boost::thread())
        {
            iocp_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 1);
            int err = GetLastError();;
            MLOG_NOTICE("port = " << iocp_ << ", error: " << err);

            thread_ = boost::thread(boost::bind(&Impl::run, this));
        }
    }

    void doSend(int id, const mstd::rc_buffer & buffer)
    {
        MLOG_DEBUG("doSend(" << id << ", " << mlog::dump(buffer.data(), buffer.size()) << ")");

        if(id)
        {
            auto i = connections_.find(id);
            if(i != connections_.end())
                (*i)->send(buffer.data(), buffer.size());
        } else
            for(auto i = connections_.begin(), end = connections_.end(); i != end; ++i)
                (*i)->send(buffer.data(), buffer.size());
    }

    void doListen(const Listener & listener, const std::wstring & name)
    {
        MLOG_DEBUG("doListen(" + mstd::utf8(name) + ")");

        listener_ = listener;
        listenName_ = name;
        startListen();
    }

    void startListen()
    {
        HANDLE handle = CreateNamedPipeW((L"\\\\.\\pipe\\" + listenName_).c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0x1000, 0x1000, 0, NULL);
        MLOG_DEBUG("startListen, handle: " << handle);

        if(handle != INVALID_HANDLE_VALUE)
        {
            PipeConnection * connection = new PipeConnection(*this, listener_, handle);
            connection->connect();
        } else {
            int err = GetLastError();
            MLOG_ERROR("create named pipe failed: " << err);
            timers_.push(timerOperation(boost::posix_time::milliseconds(250), boost::bind(&Impl::startListen, this)));
        }
    }

    void doConnect(const Listener & listener, const std::wstring & name, int retries)
    {
        MLOG_DEBUG("doConnect(" << mstd::utf8(name) << ", " << retries << ")");

        HANDLE handle = CreateFileW((L"\\\\.\\pipe\\" + name).c_str(), PIPE_ACCESS_DUPLEX, PIPE_NOWAIT, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        int err = GetLastError();
        MLOG_DEBUG("doConnect, handle: " << handle);

        if(handle != INVALID_HANDLE_VALUE) 
        {
            PipeConnection * connection = new PipeConnection(*this, listener, handle);
            connection->connectDone(0);
        } else {
            MLOG_WARNING("connect failed: " << err);
            if(retries)
            {
                if(retries > 0)
                    --retries;
                timers_.push(timerOperation(boost::posix_time::milliseconds(250), boost::bind(&Impl::doConnect, this, listener, name, retries)));
            } else {
                listener(-err, -pcFailed, 0, 0);
            }                
        }
    }

    void run()
    {
        MLOG_NOTICE("run()");

        try {
            while(!boost::this_thread::interruption_requested())
            {
                DWORD bytesTransferred;
                ULONG_PTR completionKey;
                LPOVERLAPPED overlapped;
                boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
                DWORD wait;
                {
                    TimerOperation * op;
                    while(!timers_.empty() && (op = timers_.top())->time <= now)
                    {
                        timers_.pop();
                        op->run();
                        delete op;
                    }
                    wait = timers_.empty() ? 500 : static_cast<DWORD>((op->time - now).total_milliseconds());
                }
                BOOL res = GetQueuedCompletionStatus(iocp_, &bytesTransferred, &completionKey, &overlapped, wait);
                int err = GetLastError();
                if(overlapped)
                {
                    MLOG_DEBUG("queued completion: " << overlapped << ", key: " << completionKey);
                    IoOperation * op = static_cast<IoOperation*>(overlapped);
                    op->complete(bytesTransferred, err);
                    if(completionKey)
                        delete op;
                } else {
                    if(err != WAIT_TIMEOUT)
                    {
                        MLOG_ERROR("get status failed: " << err);
                        break;
                    }
                }
            }
        } catch(boost::thread_interrupted&) {
            MLOG_NOTICE("run, interrupted");
        }

        MLOG_NOTICE("run, done");
    }

    boost::thread thread_;
    HANDLE iocp_;
    std::priority_queue<TimerOperation*, std::vector<TimerOperation*>, CompareTime> timers_;
    mstd::tid_map<mstd::tid_map_key<uint32_t>, PipeConnection*> connections_;

    std::wstring listenName_;
    Listener listener_;
};

PipeService::PipeService()
    : impl_(new Impl())
{
}

PipeService::~PipeService()
{
}

void PipeService::listen(const Listener & listener, const std::wstring & name)
{
    impl_->listen(listener, name);
}

void PipeService::connect(const Listener & listener, const std::wstring & name, int retries)
{
    impl_->connect(listener, name, retries);
}

void PipeService::send(int id, PacketCode code, const char * begin, size_t len)
{
    impl_->send(id, code, begin, len);
}

void PipeService::post(const boost::function<void()> & action)
{
    impl_->post(action);
}

void PipeService::disconnect(int id)
{
    impl_->disconnect(id);
}

}

#endif
