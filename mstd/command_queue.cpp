/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#if !defined(_STLP_NO_IOSTREAMS)

#include "reverse_lock.hpp"

#include "command_queue.hpp"

namespace mstd {

command_queue::command_queue()
{
    thread_ = boost::thread(&command_queue::execute, this);
}

command_queue::~command_queue()
{
    thread_.interrupt();
    thread_.join();
}

void command_queue::enqueue(const command_type & command)
{
    boost::unique_lock<boost::mutex> lock(mutex_);
    queue_.push_back(command);
    cond_.notify_one();
}

void command_queue::execute()
{
    queue_type queue;

    boost::unique_lock<boost::mutex> lock(mutex_);

    while(!boost::this_thread::interruption_requested())
    {
        try {
            if(queue_.empty())
                cond_.wait(lock);
            else {
                queue_.swap(queue);
                reverse_lock<boost::unique_lock<boost::mutex> > rlock(lock);
                for(queue_type::const_iterator i = queue.begin(), end = queue.end(); i != end; ++i)
                    try {
                        (*i)();
                    } catch(boost::thread_interrupted&) {
                        return;
                    } catch(...) {
                    }
                queue.clear();
            }
        } catch(boost::thread_interrupted&) {
            return;
        }
    }
}

}

#endif
