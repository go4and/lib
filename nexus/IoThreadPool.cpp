#include "pch.h"

#include "IoThreadPool.h"

MLOG_DECLARE_LOGGER(nexus_iotp);

namespace nexus {

struct IoThreadPool::Impl {
    boost::ptr_vector<boost::thread> threads;
    boost::asio::io_service ioService;
};

IoThreadPool::IoThreadPool()
    : impl_(new Impl)
{
    MLOG_MESSAGE(Notice, "<init>");
}

IoThreadPool::~IoThreadPool()
{
    MLOG_MESSAGE(Notice, "<done>");
    impl_.reset();
    MLOG_MESSAGE(Notice, "<done>, finished");
}

namespace {

class Runner {
public:
    explicit Runner(boost::asio::io_service & service)
        : service_(service) {}

    void operator()() const
    {
        MLOG_MESSAGE(Notice, "runner");
        service_.run();
        MLOG_MESSAGE(Notice, "runner, done");
    }
private:
    boost::asio::io_service & service_;
};

}

void IoThreadPool::start(size_t count, bool withCurrent)
{
    MLOG_MESSAGE(Notice, "start(" << count << ", " << withCurrent << ")");

    if(withCurrent)
        --count;
    impl_->threads.reserve(count);

    while(impl_->threads.size() != count)
        impl_->threads.push_back(new boost::thread(tracer(logger, Runner(impl_->ioService))));
    
    if(withCurrent)
        tracer(logger, Runner(impl_->ioService))();
}

void IoThreadPool::stop()
{
    MLOG_MESSAGE(Notice, "stop");

    for(boost::ptr_vector<boost::thread>::iterator i = impl_->threads.begin(), end = impl_->threads.end(); i != end; ++i)
        i->join();
    impl_->threads.clear();

    MLOG_MESSAGE(Notice, "stop, finished");
}

boost::asio::io_service & IoThreadPool::ioService()
{
    return impl_->ioService;
}

}
