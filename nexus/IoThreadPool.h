#pragma once

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
