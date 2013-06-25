/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <boost/thread/tss.hpp>

#include "singleton.hpp"

namespace mstd {

template<class T>
T & get(boost::thread_specific_ptr<T> & tss)
{
    T * result = tss.get();
    if(!result)
        tss.reset(result = new T);
    return *result;
}

namespace detail {
    template<class TSS>
    class TSSHolder : public mstd::singleton<TSSHolder<TSS> > {
    public:
        TSS & tss()
        {
            return get(tss_);
        }
    private:
        boost::thread_specific_ptr<TSS> tss_;
    };
}

template<class TSS>
TSS & tss()
{
    return detail::TSSHolder<TSS>::instance().tss();
}

}
