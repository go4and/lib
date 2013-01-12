#pragma once

#include "Defines.h"

namespace fcgi {

struct FCGIRecord;

class Connection : public mstd::reference_counter<Connection> {
public:
    explicit Connection(const nexus::isocket & socket, const RequestHandler & requestHandler);
    ~Connection();

    void send(const char * begin, const char * end);

    inline void start() { startRead(); }
    inline void send(const char * str) { send(str, str + strlen(str)); }
    inline boost::asio::io_service & ioService() { return (*socket_)->get_io_service(); }   
private:
    inline ConnectionPtr ptr()
    {
        return this;
    }

    void startRead();
    void handleRead(const boost::system::error_code & ec, size_t bt, const ConnectionPtr & ptr);
    bool processRecords();
    bool processRecord(const FCGIRecord & rec);
    void processParams();
    void handleWrite(const boost::system::error_code & ec, size_t bytes, const ConnectionPtr & conn);

    nexus::isocket socket_;
    RequestHandler requestHandler_;
    std::vector<char> buffer_;
    std::vector<char> stream_;
    std::vector<char> output_;
    size_t pos_;
    Params params_;
    RequestId requestId_;
    bool keepAlive_;

    NEXUS_DECLARE_HANDLER(Read, Connection, true);
    NEXUS_DECLARE_HANDLER(Write, Connection, true);
};


}
