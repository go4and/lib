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
    static const boost::uint32_t limit_ = 16;
    boost::uint32_t state_;
};

} }
