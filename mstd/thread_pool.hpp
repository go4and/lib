/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

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
