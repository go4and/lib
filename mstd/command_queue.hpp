/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <vector>

#include <boost/scoped_ptr.hpp>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include <boost/thread/condition_variable.hpp>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <boost/thread/thread.hpp>

#include "singleton.hpp"

namespace mstd {

class MSTD_DECL command_queue {
public:
    typedef std::function<void()> command_type;

    command_queue();
    ~command_queue();

    void enqueue(const command_type & command);
private:
    void execute();

    typedef std::vector<command_type> queue_type;

    boost::mutex              mutex_;
    boost::condition_variable cond_;
    boost::thread             thread_;
    queue_type                queue_;
};

template<class Tag>
class tagged_command_queue : public mstd::singleton<tagged_command_queue<Tag> > {
public:
    void enqueue(const command_queue::command_type & command)
    {
        impl_.enqueue(command);
    }
private:
    tagged_command_queue() {}

    command_queue impl_;

    MSTD_SINGLETON_DECLARATION(tagged_command_queue);
};

template<class Tag>
void enqueue(const command_queue::command_type & command)
{
    tagged_command_queue<Tag>::instance().enqueue(command);
}

class default_enqueue_tag;

inline void default_enqueue(const command_queue::command_type & command)
{
    enqueue<default_enqueue_tag>(command);
}

}
