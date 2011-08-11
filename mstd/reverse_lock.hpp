#pragma once

#include <boost/noncopyable.hpp>

namespace mstd {

template<class Lock>
class reverse_lock : public boost::noncopyable {
public:
    explicit reverse_lock(Lock & lock)
        : lock_(lock)
    {
        lock_.unlock();
    }
    
    ~reverse_lock()
    {
        lock_.lock();
    }
private:
    Lock & lock_;
};

}
