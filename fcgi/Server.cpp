#include "pch.h"

#include "Connection.h"

#include "Server.h"

MLOG_DECLARE_LOGGER(fcgi_server);

namespace fcgi {

Server::Server(boost::asio::io_service & ioService, const RequestHandler & handler)
    : acceptor_(ioService), handler_(handler) {}

void Server::start(unsigned short port)
{
    MLOG_MESSAGE(Debug, "start(" << port << ")");

    nexus::listen(acceptor_, port);
    startAccept();
}

void Server::stop()
{
    acceptor_.close();
}

void Server::startAccept()
{
    MLOG_MESSAGE(Debug, "startAccept()");

    nexus::isocket socket(new nexus::Socket(acceptor_.io_service()));
    acceptor_.async_accept(**socket, bindAccept(socket));
}

void Server::handleAccept(const boost::system::error_code & ec, const nexus::isocket & socket)
{
    MLOG_MESSAGE(Info, "handleAccept(" << ec << ")");

    if(!ec)
    {
        MLOG_MESSAGE(Debug, "accepted, local: " << (*socket)->local_endpoint() << ", remote: " << (*socket)->remote_endpoint());

        (*socket)->set_option(boost::asio::ip::tcp::no_delay(true));

        ConnectionPtr conn = new Connection(socket, handler_);
        conn->start();

        startAccept();
    } else {
        MLOG_MESSAGE(Warning, "Accept failed: " << ec << ", message: " << ec.message());
    }
}

}
