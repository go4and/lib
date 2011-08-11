#pragma once

#include <boost/thread/mutex.hpp>

#include "threads.hpp"

namespace mstd {

class debug_mutex {
public:
    void lock()
    {
        impl_.lock();
        owner_ = this_thread_id();
    }

    void unlock()
    {
        owner_ = 0;
        impl_.unlock();
    }

    thread_id owner() const
    {
        return owner_;
    }
private:
    boost::mutex impl_;
    volatile thread_id owner_;
};

}
