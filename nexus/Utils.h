#pragma once

#ifndef NEXUS_BUILDING

#include <boost/asio/ip/tcp.hpp>

#endif

#include "Config.h"

namespace nexus {

NEXUS_DECL void listen(boost::asio::ip::tcp::acceptor & acceptor, const boost::asio::ip::tcp::endpoint & ep);
NEXUS_DECL void listen(boost::asio::ip::tcp::acceptor & acceptor, unsigned short port);
NEXUS_DECL void setupSocket(boost::asio::ip::tcp::socket & socket, int sendBufferSize, int recvBufferSize);

}
