#pragma once

#ifndef NEXUS_BUILDING
#include <boost/asio/ip/tcp.hpp>

#include <boost/function.hpp>
#endif

#include "Handler.h"

namespace nexus {

typedef boost::function<void(boost::asio::ip::tcp::socket &)> AcceptorListener;

class Acceptor {
public:
    explicit Acceptor(boost::asio::io_service & ios, const AcceptorListener & listener);
    
    void startAnyV4(unsigned short port) { start(boost::asio::ip::address_v4::any(), port); }
    void startLoopbackV4(unsigned short port) { start(boost::asio::ip::address_v4::loopback(), port); }

    void startAnyV6(unsigned short port) { start(boost::asio::ip::address_v6::any(), port); }
    void startLoopbackV6(unsigned short port) { start(boost::asio::ip::address_v6::loopback(), port); }

    inline void start(const boost::asio::ip::address & address, unsigned short port) { start(boost::asio::ip::tcp::endpoint(address, port)); }
    void start(const boost::asio::ip::tcp::endpoint & ep);

    void startAnyV4(unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::address_v4::any(), port, ec); }
    void startLoopbackV4(unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::address_v4::loopback(), port, ec); }

    void startAnyV6(unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::address_v6::any(), port, ec); }
    void startLoopbackV6(unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::address_v6::loopback(), port, ec); }

    inline void start(const boost::asio::ip::address & address, unsigned short port, boost::system::error_code & ec) { start(boost::asio::ip::tcp::endpoint(address, port), ec); }
    void start(const boost::asio::ip::tcp::endpoint & ep, boost::system::error_code & ec);

    const boost::asio::ip::tcp::endpoint & endpoint() const { return endpoint_; }

    void cancel();
private:
    void handleAccept(const boost::system::error_code & ec);
    void startAccept();

    AcceptorListener listener_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::endpoint endpoint_;

    NEXUS_DECLARE_HANDLER(Accept, Acceptor, true);
};

}
