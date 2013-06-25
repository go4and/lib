/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

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
