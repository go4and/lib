#pragma once

#ifndef NEXUS_BUILDING
#include <boost/asio/ip/tcp.hpp>

#include <boost/function.hpp>
#endif

#include "Handler.h"
#include "Utils.h"

namespace nexus {

class BaseAcceptor {
protected:
    mlog::Logger & getLogger();
};

template<class Derived, class Protocol>
class GenericAcceptor : public BaseAcceptor {
public:
    typedef Protocol protocol_type;
    typedef typename protocol_type::acceptor acceptor_type;
    typedef typename protocol_type::endpoint endpoint_type;
    typedef typename protocol_type::socket socket_type;

    typedef boost::function<void(socket_type &)> Listener;
    typedef boost::function<void(Derived &)> AbortListener;

    explicit GenericAcceptor(boost::asio::io_service & ios, const Listener & listener)
        : listener_(listener), acceptor_(ios), socket_(ios)
    {
    }

    void listenAbort(const AbortListener & listener)
    {
        abortListener_ = listener;
    }

    void start(const boost::asio::ip::tcp::endpoint & ep)
    {
        acceptor_type temp(acceptor_.get_io_service());
        listen(temp, ep);
        acceptor_ = boost::move(temp);
        boost::system::error_code ec;
        endpoint_ = acceptor_.local_endpoint(ec);
        if(ec)
            MLOG_FMESSAGE(Warning, "failed to get local endpoint: " << ec << ", " << ec.message());
        else
            MLOG_FMESSAGE(Notice, "started[" << endpoint_ << "]");

        startAccept();
    }

    void start(const endpoint_type & ep, boost::system::error_code & ec)
    {
        acceptor_type temp(acceptor_.get_io_service());
        listen(temp, ep, ec);
        if(!ec)
        {
            acceptor_ = boost::move(temp);
            boost::system::error_code ec;
            endpoint_ = acceptor_.local_endpoint(ec);
            if(ec)
                MLOG_FMESSAGE(Warning, "failed to get local endpoint: " << ec << ", " << ec.message());
            else
                MLOG_FMESSAGE(Notice, "started[" << endpoint_ << "]");
            startAccept();
        }
    }

    const endpoint_type & endpoint() const { return endpoint_; }

    void cancel()
    {
        acceptor_.cancel();
    }

    void cancel(boost::system::error_code & ec)
    {
        acceptor_.cancel();
    }
private:
    void handleAccept(const boost::system::error_code & ec)
    {
        MLOG_FMESSAGE(Info, "handleAccept[" << endpoint_ << "](" << ec << ")");

        if(!ec)
        {
            listener_(socket_);
            if(socket_.is_open())
                socket_ = boost::move(socket_type(acceptor_.get_io_service()));
        } else if(ec == boost::asio::error::operation_aborted)
        {
            MLOG_FMESSAGE(Notice, "accept aborted[" << endpoint_ << "]");
            acceptor_.close();
            if(!abortListener_.empty())
                acceptor_.get_io_service().post(boost::bind(abortListener_, boost::ref(*static_cast<Derived*>(this))));
            return;
        }

        startAccept();
    }

    void startAccept()
    {
        acceptor_.async_accept(socket_, bindAccept());
    }

    Listener listener_;
    AbortListener abortListener_;
    acceptor_type acceptor_;
    socket_type socket_;
    endpoint_type endpoint_;

    NEXUS_DECLARE_HANDLER(Accept, GenericAcceptor, true);
};

class TcpAcceptor : public GenericAcceptor<TcpAcceptor, boost::asio::ip::tcp> {
public:
    explicit TcpAcceptor(boost::asio::io_service & ios, const Listener & listener)
        : GenericAcceptor<TcpAcceptor, boost::asio::ip::tcp>(ios, listener)
    {
    }

    void startAnyV4(unsigned short port) { start(boost::asio::ip::address_v4::any(), port); }
    void startLoopbackV4(unsigned short port) { start(boost::asio::ip::address_v4::loopback(), port); }

    void startAnyV6(unsigned short port) { start(boost::asio::ip::address_v6::any(), port); }
    void startLoopbackV6(unsigned short port) { start(boost::asio::ip::address_v6::loopback(), port); }

    inline void start(const boost::asio::ip::address & address, unsigned short port) { start(boost::asio::ip::tcp::endpoint(address, port)); }

    void startAnyV4(unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::address_v4::any(), port, ec); }
    void startLoopbackV4(unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::address_v4::loopback(), port, ec); }

    void startAnyV6(unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::address_v6::any(), port, ec); }
    void startLoopbackV6(unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::address_v6::loopback(), port, ec); }

    inline void start(const boost::asio::ip::address & address, unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::tcp::endpoint(address, port), ec); }
    using GenericAcceptor<TcpAcceptor, boost::asio::ip::tcp>::start;
};

class LocalAcceptor : public GenericAcceptor<LocalAcceptor, boost::asio::local::stream_protocol> {
public:
    explicit LocalAcceptor(boost::asio::io_service & ios, const Listener & listener)
        : GenericAcceptor<LocalAcceptor, boost::asio::local::stream_protocol>(ios, listener)
    {
    }
};

}
