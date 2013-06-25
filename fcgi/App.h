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

class AppContext {
public:
    explicit AppContext(const std::string & name, int argc, char * argv[]);
    ~AppContext();

    void run(unsigned short port, const boost::function<void()> & starter, const RequestHandler & handler);
    boost::asio::io_service & ioService();
    const std::string & dbopts();
private:
    class Impl;
    boost::scoped_ptr<Impl> impl_;
};

template<class Manager>
class Starter {
public:
    Starter(Manager & man)
        : man_(man) {}

    void operator()() const
    {
        man_.start();
    }
private:
    Manager & man_;
};

template<class Manager>
class ManagerHandler {
public:
    ManagerHandler(Manager & man)
        : man_(man) {}

    void operator()(const fcgi::RequestPtr & request, const fcgi::ConnectionPtr & conn) const
    {
        man_.handle(*request, conn);
    }
private:
    Manager & man_;
};

template<class Manager>
void run(const std::string & name, int argc, char * argv[])
{
    AppContext context(name, argc, argv);
    {
        Manager manager(context.ioService(), context.dbopts());
        context.run(manager.port(), Starter<Manager>(manager), ManagerHandler<Manager>(manager));
    }
}

}
