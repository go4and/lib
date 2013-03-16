#pragma once

#ifndef NEXUS_BUILDING

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#endif

#include "Config.h"

namespace nexus {

NEXUS_DECL void listen(boost::asio::ip::tcp::acceptor & acceptor, const boost::asio::ip::tcp::endpoint & ep);
NEXUS_DECL void listen(boost::asio::ip::tcp::acceptor & acceptor, unsigned short port);
NEXUS_DECL void listen(boost::asio::ip::tcp::acceptor & acceptor, const boost::asio::ip::tcp::endpoint & ep, boost::system::error_code & ec);
NEXUS_DECL void listen(boost::asio::ip::tcp::acceptor & acceptor, unsigned short port, boost::system::error_code & ec);

NEXUS_DECL void listen(boost::asio::local::stream_protocol::acceptor & acceptor, const boost::asio::local::stream_protocol::endpoint & ep);
NEXUS_DECL void listen(boost::asio::local::stream_protocol::acceptor & acceptor, const boost::asio::local::stream_protocol::endpoint & ep, boost::system::error_code & ec);

NEXUS_DECL void bindBroadcast(boost::asio::ip::udp::socket & socket, unsigned short port, boost::system::error_code & ec);

NEXUS_DECL void setupSocket(boost::asio::ip::tcp::socket & socket, int sendBufferSize, int recvBufferSize);

NEXUS_DECL std::string escapeXml(const std::string & input);
inline std::string escaleHtml(const std::string & input) { return escapeXml(input); }

}
