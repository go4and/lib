#pragma once

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

namespace mstd {

class thread_pool {
public:
    void enqueue(const boost::function<void()> & f);

    thread_pool(size_t threads);
    ~thread_pool();
private:
    class impl;
    boost::scoped_ptr<impl> impl_;
};

}
