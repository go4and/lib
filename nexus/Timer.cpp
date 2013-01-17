#include "pch.h"

#include "Timer.h"

namespace nexus {

namespace {

class Manager {
    MSTD_SINGLETON_INLINE_DEFINITION(Manager);
public:
    void schedule(const mstd::command_queue::command_type & command, const boost::posix_time::time_duration & delay)
    {
        service_.post(boost::bind(&Manager::doSchedule, this, QueueItem(command, boost::posix_time::microsec_clock::universal_time() + delay)));
    }
    
    ~Manager()
    {
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            stopped_ = true;
            timer_.expires_from_now(boost::posix_time::time_duration());
        }
        thread_.join();
    }
private:
    Manager()
        : work_(service_), timer_(service_)
    {
        thread_ = boost::thread(boost::bind(&boost::asio::io_service::run, &service_));
    }

    void doSchedule(const QueueItem & item)
    {
        queue_.push(item);
        processQueue();
    }
    
    void processQueue()
    {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
        while(!queue_.empty() && queue_.top().second <= now)
        {
            mstd::default_enqueue(queue_.top().first);
            queue_.pop();
        }
        if(!queue_.empty())
        {
            boost::asio::deadline_timer * timer = new boost::asio::deadline_timer(service_, i.second);
            timer_.expires_at(queue_.top().second);
            timer->async_wait(boost::bind(&Manager::handleTimer, this, _1));
        }
    }
    
    void handleTimer(const boost::system::error_code & ec)
    {
        if(!ec)
            processQueue();
    }

    typedef std::pair<mstd::command_queue::command_type, boost::posix_time::ptime> QueueItem;
    
    struct QueueItemCompare {
        bool operator()(const QueueItem & lhs, const QueueItem & rhs) const
        {
            return lhs.second > rhs.second;
        }
    };
    
    typedef std::priority_queue<QueueItem, std::vector<QueueItem>, QueueItemCompare> Queue;

    boost::thread        thread_;
    boost::asio::io_service     service_;
    boost::asio::work           work_;
    boost::asio::deadline_timer timer_;
    Queue queue_;
};

}

void schedule(const mstd::command_queue::command_type & command, const boost::posix_time::time_duration & delay)
{
    Manager::instance().schedule(command, delay);
}

}
