/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#if !_STLP_NO_IOSTREAMS
#include <deque>
#include <vector>

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "reverse_lock.hpp"

#include "thread_pool.hpp"

namespace mstd {

class thread_pool::impl {
public:
    explicit impl(size_t t)
        : finished_(false)
    {
        for(size_t i = 0; i != t; ++i)
            threads_.push_back(boost::shared_ptr<boost::thread>(new boost::thread(std::bind(&impl::execute, this))));
    }

    void enqueue(const std::function<void()> & f)
    {
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            queue_.push_back(f);
        }
        cond_.notify_one();
    }
    
    ~impl()
    {
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            finished_ = true;
        }
        cond_.notify_all();
        for(Threads::const_iterator i = threads_.begin(), end = threads_.end(); i != end; ++i)
            (*i)->join();
    }
private:
    void execute()
    {
        try {
            boost::unique_lock<boost::mutex> lock(mutex_);
            while(!finished_ && !boost::this_thread::interruption_requested())
            {
                cond_.timed_wait(lock, boost::posix_time::milliseconds(100));
                if(!queue_.empty())
                {
                    std::function<void()> f = queue_.front();
                    queue_.pop_front();
                    reverse_lock<boost::unique_lock<boost::mutex> > rlock(lock);
                    try {
                        f();
                    } catch(boost::thread_interrupted&) {
                        break;
                    } catch(...) {
                    }
                }
            }
        } catch(boost::thread_interrupted&) {
        }
    }

    typedef std::vector<boost::shared_ptr<boost::thread> > Threads;
    Threads threads_;
    boost::condition_variable cond_;
    boost::mutex mutex_;
    bool finished_;
    std::deque<std::function<void()> > queue_;
};

thread_pool::thread_pool(size_t threads)
    : impl_(new impl(threads))
{
}

thread_pool::~thread_pool()
{
}

void thread_pool::enqueue(const std::function<void()> & f)
{
    impl_->enqueue(f);
}

}
#endif
