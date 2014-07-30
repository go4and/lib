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

#include "PacketReader.h"
#include "PipeNode.h"

MLOG_DECLARE_LOGGER(pipenode);

namespace nexus {

PipeNode::PipeNode()
    : Connection(false, 1 << 8, 2), timer_(ioService_)
{
}

PipeNode::~PipeNode()
{
    thread_.interrupt();
    ioService_.stop();
    thread_.join();
    pipe_.reset();
}

class RunIOService {
public:
    explicit RunIOService(boost::asio::io_service & ioService)
        : ioService_(ioService) {}

    void operator()()
    {
        ioService_.run();
    }
private:
    boost::asio::io_service & ioService_;
};

void PipeNode::start(const Listener & listener)
{
    listener_ = listener;
    thread_ = move(boost::thread(mlog::tracer(::logger, RunIOService(ioService_), true)));
}

void PipeNode::listen(const std::wstring & name)
{
    ioService_.dispatch(std::bind(&PipeNode::doListen, this, name));
}

void PipeNode::doListen(const std::wstring & name)
{
    HANDLE handle = CreateNamedPipeW((L"\\\\.\\pipe\\" + name).c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT,
                                    PIPE_UNLIMITED_INSTANCES, 0x1000, 0x1000, 0, NULL);
    pipe_.reset(new boost::asio::windows::stream_handle(ioService_, handle));
    boost::asio::windows::overlapped_ptr overlapped(ioService_, std::bind(&PipeNode::handleConnected, this, std::placeholders::_1, name));

    bool ok = ConnectNamedPipe(pipe_->native(), overlapped.get()) != FALSE;
    DWORD error = GetLastError();

    if(!ok && error != ERROR_IO_PENDING)
    {
        boost::system::error_code ec(error, boost::asio::error::get_system_category());
        overlapped.complete(ec, 0);
    } else {
        overlapped.release();
    }
}

void PipeNode::connect(const std::wstring & name)
{
    ioService_.dispatch(std::bind(&PipeNode::doConnect, this, name));
}

void PipeNode::doConnect(const std::wstring & name)
{
    HANDLE handle = CreateFileW((L"\\\\.\\pipe\\" + name).c_str(), PIPE_ACCESS_DUPLEX, PIPE_WAIT, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if(handle != INVALID_HANDLE_VALUE) 
    {
        pipe_.reset(new boost::asio::windows::stream_handle(ioService_, handle));
        connectDone();
    } else {
        timer_.expires_from_now(boost::posix_time::milliseconds(250));
        timer_.async_wait(std::bind(&PipeNode::handleExpired, this, std::placeholders::_1, [this, name](){ doConnect(name); }));
    }
}

void PipeNode::connectDone()
{
    if(activate())
    {
        listener_(pcConnected, PacketReader());
        Connection::start();
    }
}

void PipeNode::handleExpired(const boost::system::error_code & ec, const std::function<void()> & action)
{
    if(!ec)
        action();
}

void PipeNode::handleConnected(const boost::system::error_code & ec, const std::wstring & name)
{
    if(!ec)
    {
        MLOG_MESSAGE(Notice, "Pipe connected");
        connectDone();
    } else if(ec != boost::asio::error::broken_pipe) {
        MLOG_MESSAGE(Warning, "Listen failed: " << ec << ", " << ec.message());
        timer_.expires_from_now(boost::posix_time::milliseconds(250));
        timer_.async_wait(std::bind(&PipeNode::handleExpired, this, std::placeholders::_1, [this, name]() { doListen(name); }));
    }
}

boost::asio::windows::stream_handle & PipeNode::stream()
{
    return *pipe_;
}

void PipeNode::finish()
{
    pipe_.reset();

    listener_(nexus::pcDisconnected, nexus::PacketReader());
}

void PipeNode::shutdown()
{
    boost::system::error_code ec;
    pipe_->close(ec);
}

void PipeNode::processPackets(PacketReader & reader)
{
    while(reader.left() >= 3)
    {
        reader.mark();
        PacketCode code = reader.read<PacketCode>();
        boost::uint32_t len = reader.read<boost::uint16_t>();
        if(len > 0x7fff)
        {
            if(reader.left() >= 2)
                len = (reader.read<uint16_t>() << 15) | (len & 0x7fff);
        }
        if(len > reader.left())
        {
            reader.revert();
            break;
        }
        listener_(code, reader.subreader(0, len));
        reader.skip(len);
    }
}

bool PipeNode::connected()
{
    return Connection::active();
}

}

#endif
