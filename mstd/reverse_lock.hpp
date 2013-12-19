/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <exception>
#include <boost/noncopyable.hpp>

namespace mstd {

class reverse_lock_unlocked_exception : public std::exception {
public:
    reverse_lock_unlocked_exception()
    {
    }
    
    virtual const char* what() const throw()
    {
        return "lock did not locked in reverse lock";
    }
};

template<class Lock>
class reverse_lock : public boost::noncopyable {
public:
    explicit reverse_lock(Lock & lock)
        : lock_(lock)
    {
        if(lock_.owns_lock())
            lock_.unlock();
        else
            throw reverse_lock_unlocked_exception();
    }
    
    ~reverse_lock()
    {
        lock_.lock();
    }
private:
    Lock & lock_;
};

}
