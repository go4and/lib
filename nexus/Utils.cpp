#include "pch.h"

#include "Buffer.h"

#include "Utils.h"

MLOG_DECLARE_LOGGER(nexus_utils);

namespace nexus {

void listen(boost::asio::ip::tcp::acceptor & acceptor, unsigned short port)
{
    listen(acceptor, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
}

void listen(boost::asio::ip::tcp::acceptor & acceptor, const boost::asio::ip::tcp::endpoint & endpoint)
{
    MLOG_MESSAGE(Debug, "listen(" << endpoint << ')');

    try {
        acceptor.open(endpoint.protocol());
        acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor.bind(endpoint);
        acceptor.listen();
    } catch (boost::exception & e) {
        MLOG_MESSAGE(Error, "Listen " << endpoint << " failed: " << mstd::out_exception(e));
        throw;
    } catch (std::exception & e) {
        MLOG_MESSAGE(Error, "Listen " << endpoint << " failed: " << mstd::out_exception(e));
        throw;
    }
}

void setupSocket(boost::asio::ip::tcp::socket & socket, int sendBufferSize, int recvBufferSize)
{
    boost::system::error_code ec;

    if(logger.enabled(mlog::llInfo))
    {
        boost::asio::ip::tcp::socket::send_buffer_size sends;
        if(socket.get_option(sends, ec))
            MLOG_MESSAGE(Error, "get send_buffer_size failed: " << ec << ", " << ec.message());
        boost::asio::ip::tcp::socket::receive_buffer_size recvs;
        if(socket.get_option(recvs, ec))
            MLOG_MESSAGE(Error, "get send_buffer_size failed: " << ec << ", " << ec.message());

        MLOG_MESSAGE(Info, "old socket options, send: " << sends.value() << ", recv: " << recvs.value());
    }

    if(socket.set_option(boost::asio::ip::tcp::no_delay(true), ec))
        MLOG_MESSAGE(Error, "set no_delay failed: " << ec << ", " << ec.message());

    if(socket.set_option(boost::asio::ip::tcp::socket::send_buffer_size(sendBufferSize), ec))
        MLOG_MESSAGE(Error, "set send_buffer_size failed: " << ec << ", " << ec.message());
    if(socket.set_option(boost::asio::ip::tcp::socket::receive_buffer_size(recvBufferSize), ec))
        MLOG_MESSAGE(Error, "set receive_buffer_size failed: " << ec << ", " << ec.message());

    if(logger.enabled(mlog::llInfo))
    {
        boost::asio::ip::tcp::socket::send_buffer_size sends;
        if(socket.get_option(sends, ec))
            MLOG_MESSAGE(Error, "get send_buffer_size failed: " << ec << ", " << ec.message());
        boost::asio::ip::tcp::socket::receive_buffer_size recvs;
        if(socket.get_option(recvs, ec))
            MLOG_MESSAGE(Error, "get send_buffer_size failed: " << ec << ", " << ec.message());

        MLOG_MESSAGE(Info, "new socket options, send: " << sends.value() << ", recv: " << recvs.value());
    }
}

}
