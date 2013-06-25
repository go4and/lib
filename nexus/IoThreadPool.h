/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

namespace boost { namespace asio {
    class io_service;
} }

namespace nexus {

class IoThreadPool {
public:
    IoThreadPool();
    ~IoThreadPool();

    boost::asio::io_service & ioService();

    void start(size_t count, bool withCurrent = false);
    void stop();
private:
    struct Impl;

    boost::scoped_ptr<Impl> impl_;
};

}
