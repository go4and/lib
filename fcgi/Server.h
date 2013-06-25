/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include "Defines.h"

namespace fcgi {

class Server {
public:
    Server(boost::asio::io_service & ioService, const RequestHandler & handler);

    void start(unsigned short port);
    void stop();
private:
    void startAccept();
    void handleAccept(const boost::system::error_code & ec, const nexus::isocket & socket);

    boost::asio::ip::tcp::acceptor acceptor_;
    RequestHandler handler_;

    NEXUS_DECLARE_HANDLER(Accept, Server, true);
};

}
