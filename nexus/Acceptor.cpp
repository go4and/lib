#include "pch.h"

#include "Utils.h"

#include "Acceptor.h"

MLOG_DECLARE_LOGGER(nexus_acceptor);

namespace nexus {

Acceptor::Acceptor(boost::asio::io_service & ios, const AcceptorListener & listener)
    : listener_(listener), acceptor_(ios), socket_(ios)
{
}

void Acceptor::start(const boost::asio::ip::tcp::endpoint & ep)
{
    listen(acceptor_, ep);
    boost::system::error_code ec;
    endpoint_ = acceptor_.local_endpoint(ec);
    if(ec)
        MLOG_WARNING("failed to get local endpoint: " << ec << ", " << ec.message());
    else
        MLOG_NOTICE("started[" << endpoint_ << "]");

    startAccept();
}

void Acceptor::handleAccept(const boost::system::error_code & ec)
{
    MLOG_INFO("handleAccept[" << endpoint_ << "](" << ec << ")");

    if(!ec)
    {
        listener_(socket_);
        if(socket_.is_open())
            socket_ = boost::move(boost::asio::ip::tcp::socket(acceptor_.get_io_service()));
    } else if(ec == boost::asio::error::operation_aborted)
    {
        MLOG_NOTICE("accept aborted[" << endpoint_ << "]");
        return;
    }

    startAccept();
}

void Acceptor::startAccept()
{
    acceptor_.async_accept(socket_, bindAccept());
}

void Acceptor::cancel()
{
    acceptor_.cancel();
}

}
