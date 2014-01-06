/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Buffer.h"

#include "Utils.h"

MLOG_DECLARE_LOGGER(nexus_utils);

namespace nexus {

namespace {

const char * const AMP = "&amp;";
const char * const QUOT = "&quot;";
const char * const GT = "&gt;";
const char * const LT = "&lt;";

}

void listen(boost::asio::ip::tcp::acceptor & acceptor, unsigned short port)
{
    listen(acceptor, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));
}

void listen(boost::asio::ip::tcp::acceptor & acceptor, const boost::asio::ip::tcp::endpoint & endpoint)
{
    MLOG_MESSAGE(Debug, "listen(" << endpoint << ')');

    try {
        boost::asio::ip::tcp::endpoint::protocol_type type = endpoint.protocol();
        acceptor.open(type);
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

void listen(boost::asio::ip::tcp::acceptor & acceptor, const boost::asio::ip::tcp::endpoint & endpoint, boost::system::error_code & ec)
{
    MLOG_MESSAGE(Debug, "listen(" << endpoint << "), acceptor size: " << sizeof(acceptor) << ", impl size: " << sizeof(boost::asio::ip::tcp::acceptor::implementation_type));

    acceptor.open(endpoint.protocol(), ec);
    if(!ec)
    {
        acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
        if(!ec)
        {
            acceptor.bind(endpoint, ec);
            if(!ec)
            {
                acceptor.listen(boost::asio::ip::tcp::socket::max_connections, ec);
                if(!ec)
                    return;
                else
                    MLOG_ERROR("Listen " << endpoint << ", listen failed: " << ec << ", message: " << ec);
            } else
                MLOG_ERROR("Listen " << endpoint << ", bind failed: " << ec << ", message: " << ec);
        } else
            MLOG_ERROR("Listen " << endpoint << ", set option failed: " << ec << ", message: " << ec);
    } else
        MLOG_ERROR("Listen " << endpoint << ", open failed: " << ec << ", message: " << ec << ", handle: " << acceptor.native_handle());
}

void listen(boost::asio::ip::tcp::acceptor & acceptor, unsigned short port, boost::system::error_code & ec)
{
    listen(acceptor, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port), ec);
}

#if !BOOST_WINDOWS
void listen(boost::asio::local::stream_protocol::acceptor & acceptor, const boost::asio::local::stream_protocol::endpoint & endpoint)
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

void listen(boost::asio::local::stream_protocol::acceptor & acceptor, const boost::asio::local::stream_protocol::endpoint & endpoint, boost::system::error_code & ec)
{
    MLOG_MESSAGE(Debug, "listen(" << endpoint << "), acceptor size: " << sizeof(acceptor) << ", impl size: " << sizeof(boost::asio::ip::tcp::acceptor::implementation_type));

    acceptor.open(endpoint.protocol(), ec);
    if(!ec)
    {
        acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
        if(!ec)
        {
            acceptor.bind(endpoint, ec);
            if(!ec)
            {
                acceptor.listen(boost::asio::ip::tcp::socket::max_connections, ec);
                if(!ec)
                    return;
                else
                    MLOG_ERROR("Listen " << endpoint << ", listen failed: " << ec << ", message: " << ec);
            } else
                MLOG_ERROR("Listen " << endpoint << ", bind failed: " << ec << ", message: " << ec);
        } else
            MLOG_ERROR("Listen " << endpoint << ", set option failed: " << ec << ", message: " << ec);
    } else
        MLOG_ERROR("Listen " << endpoint << ", open failed: " << ec << ", message: " << ec << ", handle: " << acceptor.native_handle());
}
#endif

void bindBroadcast(boost::asio::ip::udp::socket & socket, unsigned short port, boost::system::error_code & ec)
{
    socket.open(boost::asio::ip::udp::v4(), ec);
    if(!ec)
    {
        socket.set_option(boost::asio::ip::udp::socket::broadcast(true), ec);
        if(!ec)
        {
            boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address_v4::any(), port);
            socket.bind(endpoint, ec);
            if(!ec)
            {
                MLOG_DEBUG("Bind broadcast done: " << endpoint);
                return;
            } else
                MLOG_ERROR("Bind broadcast failed bind: " << endpoint << ", ec: " << ec);
        } else
            MLOG_ERROR("Bind broadcast failed set option: " << ec);
    } else
        MLOG_ERROR("Bind broadcast failed open: " << ec);
}

void setupSocket(boost::asio::ip::tcp::socket & socket, int sendBufferSize, int recvBufferSize)
{
    boost::system::error_code ec;

#if !defined(MLOG_NO_LOGGING)
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
#endif

    if(socket.set_option(boost::asio::ip::tcp::no_delay(true), ec))
        MLOG_MESSAGE(Error, "set no_delay failed: " << ec << ", " << ec.message());

    if(socket.set_option(boost::asio::ip::tcp::socket::send_buffer_size(sendBufferSize), ec))
        MLOG_MESSAGE(Error, "set send_buffer_size failed: " << ec << ", " << ec.message());
    if(socket.set_option(boost::asio::ip::tcp::socket::receive_buffer_size(recvBufferSize), ec))
        MLOG_MESSAGE(Error, "set receive_buffer_size failed: " << ec << ", " << ec.message());

#if !defined(MLOG_NO_LOGGING)
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
#endif
}

std::string escapeXml(const std::string & input)
{
    const char * p = input.c_str(), * i = p;
    std::string result;
    for(; *i; ++i)
    {
        char ch = *i;
        const char * img = ch == '&' ? AMP : ch == '"' ? QUOT : ch == '>' ? GT : ch == '<' ? LT : NULL;
        if(img)
        {
            if(i != p)
                result.append(p, i - p);
            result += img;
            p = i + 1;
        }
    }
    if(i != p)
        result.append(p, i - p);
    return result;
}

int changeUser(const std::string & user)
{
#ifndef BOOST_WINDOWS
    MLOG_NOTICE("Use user: " << user);
    passwd * pw = getpwnam(user.c_str());
    if(pw)
    {
        int res = initgroups(user.c_str(), pw->pw_gid);
        if (res)
        {
            int err = errno;
            MLOG_MESSAGE(Error, "Cannot init group: " << err << ", group: " << pw->pw_gid);
            return 2;
        }

        res = chroot(".");
        if(res)
        {
            MLOG_MESSAGE(Error, "Cannot chroot: " << res);
            return 3;
        }

        res = setregid(pw->pw_gid, pw->pw_gid);
        if(res)
        {
            MLOG_MESSAGE(Error, "Cannot setregid: " << res);
            return 4;
        }

        res = setreuid(pw->pw_uid, pw->pw_uid);
        if(res)
        {
            MLOG_MESSAGE(Error, "Cannot setreuid: " << res);
            return 4;
        }
    } else {
        MLOG_MESSAGE(Error, "Cannon find user: " << user);
        return 1;
    }
#endif
    return 0;
}

void fork()
{
#if BOOST_WINDOWS
    MLOG_ERROR("Fork is not supported under Windows");
    exit(1);
#else
    int err = ::fork();
    if(err < 0)
    {
        MLOG_MESSAGE(Error, "Fork failed: " << err);
        exit(1);
    }
    if(err > 0)
        exit(0);
    MLOG_NOTICE("Fork success");
#endif
}

}
