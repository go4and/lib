#pragma once

namespace mstd {

class spinlock {
public:
    spinlock()
        : v_(0)
    {
    }
public:
    bool try_lock()
    {
        int r = __sync_lock_test_and_set(&v_, 1);
        return r == 0;
    }

    void lock()
    {
        for(size_t k = 0; !try_lock(); ++k)
            detail::yield(k);
    }

    void unlock()
    {
        __sync_lock_release( &v_ );
    }
private:
    int v_;
};

}
