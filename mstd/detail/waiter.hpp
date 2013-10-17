/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace mstd { namespace detail {

class waiter {
public:
    waiter()
        : state_(1) {}

    void wait();

    void reset()
    {
        state_ = 1;
    }
private:
    boost::uint32_t state_;
};

} }
