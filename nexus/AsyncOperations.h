/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef NEXUS_BUILDING

#include <boost/noncopyable.hpp>

#endif

#include "Config.h"

namespace nexus {

class NEXUS_DECL AsyncOperations : public boost::noncopyable {
public:
    explicit AsyncOperations(bool active)
        : asyncOperations_(0), shuttingDown_(active ? 0 : 1) {}

    bool finish()
    {
        bool result = !asyncOperations_.cas(finishing * 2, 0);
        if(result)
            shutdown();
        return result;
    }
    
    int count()
    {
        return asyncOperations_;
    }

    bool shutdown()
    {
        return !shuttingDown_.cas(1, 0);
    }

    bool prepare()
    {
        if(active())
            return ++asyncOperations_ <= finishing;
        else
            return false;
    }
    
    boost::uint32_t complete()
    {
        return --asyncOperations_;
    }
    
    bool active()
    {
        return !shuttingDown_;
    }
    
    bool activate()
    {
        boost::uint32_t old = shuttingDown_.cas(0, 1);
        if(old)
        {
            asyncOperations_ = 0;
            return true;
        } else
            return false;
    }
private:
    mstd::atomic<boost::uint32_t> asyncOperations_;
    mstd::atomic<boost::uint32_t> shuttingDown_;

    static const boost::uint32_t finishing = 0xffff;
};

}
