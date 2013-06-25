/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#ifndef NEXUS_BUILDING

#include <boost/noncopyable.hpp>

#include <boost/thread/tss.hpp>

#include <mstd/pointer_cast.hpp>
#include <mstd/singleton.hpp>

#endif

#include "Config.h"

namespace nexus {

    namespace detail {

        template<class T>
        class NEXUS_DECL TSS : public mstd::singleton<TSS<T> > {
        public:
            boost::thread_specific_ptr<T> & ptr()
            {
                return ptr_;
            }
        private:
            static void null_deleter(T *)
            {
            }

            TSS()
                : ptr_(&null_deleter) {}

            boost::thread_specific_ptr<T> ptr_;
            
            MSTD_SINGLETON_DECLARATION(TSS<T>);
        };

    }

struct NoAsyncData;

template<class T>
class AsyncGuard : public T, public boost::noncopyable {
public:
    using T::asyncOperations;
    using T::finish;
    using T::shutdown;

    template<class U>
    explicit AsyncGuard(U * u, typename T::AsyncData data)
        : T(u, data), failed_(false)
    {
        BOOST_ASSERT(!tss.ptr().get());
        tss.ptr().reset(this);
    }

    template<class U>
    explicit AsyncGuard(U * u)
        : T(u, typename T::AsyncData()), failed_(false)
    {
        BOOST_STATIC_ASSERT((boost::is_same<typename T::AsyncData, NoAsyncData>::value));
        BOOST_ASSERT(!tss.ptr().get());
        tss.ptr().reset(this);
    }

    void failed()
    {
        failed_ = true;
    }
    
    ~AsyncGuard()
    {
        boost::uint32_t value = asyncOperations().complete();
        if(failed_)
            tryFinish(true);
        else if(!value)
            tryFinish(false);

        BOOST_ASSERT(tss.ptr().get() == this);
        tss.ptr().reset(0);
    }
    
    static AsyncGuard * get()
    {
        return tss.ptr().get();
    }
private:
    void tryFinish(bool s)
    {
        if(asyncOperations().finish())
            finish();
        else if(s)
            stop();
    }

    void stop()
    {
        if(asyncOperations().shutdown())
            shutdown();
    }

    static detail::TSS<AsyncGuard<T> > & tss;

    bool failed_;
};

template<class T>
detail::TSS<AsyncGuard<T> > & AsyncGuard<T>::tss = detail::TSS<AsyncGuard<T> >::instance();

}
