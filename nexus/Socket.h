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

#include <boost/noncopyable.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/asio/ip/tcp.hpp>

#include <mstd/reference_counter.hpp>

#endif

#include "Config.h"

namespace nexus {

class NEXUS_DECL Socket : public boost::noncopyable, public mstd::reference_counter<Socket> {
public:
    explicit Socket(boost::asio::io_service & ioService)
        : socket_(ioService) {}

    ~Socket()
    {
    }

    boost::asio::ip::tcp::socket * operator->()
    {
        return &socket_;
    }
    
    boost::asio::ip::tcp::socket & operator*()
    {
        return socket_;
    }

    template <typename ConstBufferSequence, typename WriteHandler>
    void async_send(const ConstBufferSequence& buffers, WriteHandler handler)
    {
        socket_.async_send(buffers, handler);
    }
private:
    boost::asio::ip::tcp::socket socket_;
};

typedef boost::intrusive_ptr<Socket> isocket;

}
