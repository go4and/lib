#include <stdlib.h>

#include <vector>

#include <boost/config.hpp>
#include <boost/version.hpp>

#if !BOOST_WINDOWS
#include <pthread.h>
#endif

#include <boost/interprocess/detail/atomic.hpp>

#include "singleton.hpp"

namespace mstd { 

void MSTD_STDCALL call_once(once_flag & flag, void (MSTD_STDCALL *f)())
{
#if BOOST_VERSION < 104800
    namespace bid = boost::interprocess::detail;
#else
    namespace bid = boost::interprocess::ipcdetail;
#endif
    if(bid::atomic_read32(&flag) != 1)
    {
        if(bid::atomic_cas32(&flag, 2, 0) == 0)
        {
            f();
            bid::atomic_write32(&flag, 1);
        } else {
            while(bid::atomic_read32(&flag) != 1)
            {
#if BOOST_WINDOWS
                SwitchToThread();
#elif defined(__APPLE__) || defined(ANDROID)
                sched_yield();
#else
                pthread_yield();
#endif
            }
        }
    }
}

namespace detail {

namespace {

typedef void (MSTD_STDCALL *cleaner_type)();
typedef std::vector<cleaner_type> Cleaners;

Cleaners* cleaners_;
once_flag once_ = MSTD_ONCE_INIT;
mstd::mutex * mutex_ = 0;

void cleanup()
{
    for(Cleaners::const_reverse_iterator i = cleaners_->rbegin(), end = cleaners_->rend(); i != end; ++i)
    {
#ifndef BOOST_NO_EXCEPTIONS
        try {
#endif
            (*i)();
#ifndef BOOST_NO_EXCEPTIONS
        } catch(...) {
        }
#endif
    }
    delete cleaners_;
    delete mutex_;
}

template<class T>
class Cleanup {
public:
    Cleanup(T * t)
        : t_(t) {}

    void operator()() const
    {
        delete t_;
    }
private:
    T * t_;
};

void MSTD_STDCALL init_cleaners()
{
    cleaners_ = new Cleaners;
    mutex_ = new mstd::mutex;
    atexit(&cleanup);
}

}

void MSTD_STDCALL register_cleaner(void (MSTD_STDCALL *cleaner)())
{
    call_once(once_, &init_cleaners);

    mstd::lock_guard<mstd::mutex> lock(*mutex_);
    cleaners_->push_back(cleaner);
}

} }
