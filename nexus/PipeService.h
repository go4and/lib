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
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#endif

#include "Packet.h"

namespace nexus {

class PipeService : boost::noncopyable {
public:
    typedef boost::function<void(int, int, const char *, size_t)> Listener;

    explicit PipeService();
    ~PipeService();

    void listen(const Listener & listener, const std::wstring & name);
    void connect(const Listener & listener, const std::wstring & name, int retries = -1);

    void send(int id, PacketCode code, const char * begin, const char * end)
    {
        send(id, code, begin, end - begin);
    }

    void send(int id, PacketCode code, const char * begin, size_t len);

    void disconnect(int id);

    void post(const boost::function<void()> & action);
private:
    class Impl;
    boost::scoped_ptr<Impl> impl_;
};

}
