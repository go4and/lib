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
