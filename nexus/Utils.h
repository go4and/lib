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

#if !BOOST_WINDOWS
NEXUS_DECL void listen(boost::asio::local::stream_protocol::acceptor & acceptor, const boost::asio::local::stream_protocol::endpoint & ep);
NEXUS_DECL void listen(boost::asio::local::stream_protocol::acceptor & acceptor, const boost::asio::local::stream_protocol::endpoint & ep, boost::system::error_code & ec);
#endif

NEXUS_DECL void bindBroadcast(boost::asio::ip::udp::socket & socket, unsigned short port, boost::system::error_code & ec);

NEXUS_DECL void setupSocket(boost::asio::ip::tcp::socket & socket, int sendBufferSize, int recvBufferSize);

NEXUS_DECL std::string escapeXml(const std::string & input);
inline std::string escaleHtml(const std::string & input) { return escapeXml(input); }

void fork();
int changeUser(const std::string & user);

template<class Endpoint, class Str>
NEXUS_DECL Endpoint parseEndpoint(const Str & input, boost::system::error_code & ec)
{
    auto begin = input.begin(), end = input.end();
    auto pos = std::find(begin, end, ':');
    if(pos == end)
        return Endpoint(boost::asio::ip::address_v4::any(), mstd::str2int10<unsigned short>(begin, end));
    else
        return Endpoint(boost::asio::ip::address::from_string(std::string(begin, pos), ec), mstd::str2int10<unsigned short>(pos + 1, end));
}


template<class Endpoint>
inline std::size_t endpointHashValue(const Endpoint & endpoint)
{
    std::size_t seed = boost::hash_value(endpoint.port());
    const auto & address = endpoint.address();
    if(address.is_v4())
        boost::hash_combine(seed, address.to_v4().to_ulong());
    else
        boost::hash_combine(seed, address.to_v6().to_bytes());
    
    return seed;
}

class EndpointHasher {
public:
    typedef std::size_t result_type;

    template<class Endpoint>
    std::size_t operator()(const Endpoint & endpoint) const
    {
        return endpointHashValue(endpoint);
    }
};

}
