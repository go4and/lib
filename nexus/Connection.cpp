/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Connection.h"

MLOG_DECLARE_LOGGER(nexus_conn);

namespace nexus {

mstd::atomic<size_t> activeConnections_;
mstd::atomic<size_t> allocatedConnections_;

ConnectionBase::ConnectionBase(bool active, size_t readingBuffer, size_t threshold)
    : asyncOperations_(active), rbuffer_(readingBuffer), rpos_(0), threshold_(threshold),
      reads_(0), writes_(0), reading_(true), stopReason_(srNone), lastRead_(Clock::milliseconds()), lastWrite_(lastRead_)
{
    ++allocatedConnections_;
    ++activeConnections_;
}

ConnectionBase::~ConnectionBase()
{
    --activeConnections_;
}

size_t ConnectionBase::activeConnections()
{
    return activeConnections_;
}

size_t ConnectionBase::allocatedConnections()
{
    return allocatedConnections_;
}

bool ConnectionBase::active()
{
    return asyncOperations_.active();
}

int ConnectionBase::operations()
{
    return asyncOperations_.count();
}

size_t ConnectionBase::reads()
{
    return reads_;
}

size_t ConnectionBase::writes()
{
    return writes_;
}

int ConnectionBase::rpos()
{
    return rpos_;
}

mstd::thread_id ConnectionBase::lastLocker()
{
    return lastLocker_;
}

void ConnectionBase::stopReason(StopReason reason, const boost::system::error_code & ec)
{
    if(stopReason_.cas(reason, srNone) == srNone)
        ec_ = ec;
}

void ConnectionBase::stopReading(StopReason reason, const boost::system::error_code & ec)
{
    stopReason(reason, ec);
    reading_ = false;
}

bool ConnectionBase::reading()
{
    return reading_;
}

bool ConnectionBase::activate()
{
    return asyncOperations_.activate();
}

bool ConnectionBase::prepare()
{
    return asyncOperations_.prepare();
}

void ConnectionBase::commitWrite(size_t len, ConnectionLock &)
{
    pending_.erase(len);
}

mlog::Logger & ConnectionBase::getLogger()
{
    return logger;
}

}
