#pragma once

#ifndef NEXUS_BUILDING

#include <boost/thread/thread.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/windows/stream_handle.hpp>

#endif

#include "Connection.h"
#include "PacketPacker.h"

#include "Packet.h"

namespace nexus {

class Buffer;

class NEXUS_DECL PipeNode : public boost::noncopyable, public Connection<PipeNode> {
public:
    typedef boost::function<void(PacketCode, PacketReader)> Listener;

    PipeNode();
    ~PipeNode();

    using Connection<PipeNode>::send;

    void send(nexus::PacketCode code)
    {
        send(nexus::packCSD(code));
    }

#define NEXUS_PIPE_NODE_SEND_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    void send(nexus::PacketCode code, BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & x)) \
    { \
        size_t len = nexus::tupleSize(BOOST_PP_ENUM_PARAMS(n, x)); \
        send(nexus::packCSD(code, len, BOOST_PP_ENUM_PARAMS(n, x))); \
    } \
    /**/

    BOOST_PP_REPEAT_FROM_TO(
        1, BOOST_PP_INC(NEXUS_PACKET_PACKER_MAX_ARITY),
        NEXUS_PIPE_NODE_SEND_DEF, ~)

    void start(const Listener & listener);
    void listen(const std::wstring & name);
    void connect(const std::wstring & name);
    
    bool connected();    
private:
    void finish();
    void shutdown();
    boost::asio::windows::stream_handle & stream();
    void processPackets(nexus::PacketReader & reader);

    void doListen(const std::wstring & name);
    void doConnect(const std::wstring & name);
    void handleConnected(const boost::system::error_code & ec, const std::wstring & name);
    void handleExpired(const boost::system::error_code & ec, const boost::function<void()> & action);

    void connectDone();

    boost::asio::io_service ioService_;
    boost::scoped_ptr<boost::asio::windows::stream_handle> pipe_;
    boost::thread thread_;
    boost::asio::deadline_timer timer_;
    Listener listener_;
    
    friend class Connection<PipeNode>;
};

}
