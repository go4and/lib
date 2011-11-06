#pragma once

namespace mstd {

struct delete_disposer {
    template<class T>
    void operator()(T t) const
    {
        delete t;
    }
};

template<class Pool>
class object_pool_disposer_t {
public:
    explicit object_pool_disposer_t(Pool & pool)
        : pool_(pool) {}

    template<class T>
    void operator()(T * t) const
    {
        pool_.destroy(t);
    }
private:
    Pool & pool_;
};

template<class Pool>
object_pool_disposer_t<Pool> object_pool_disposer(Pool & pool)
{
    return object_pool_disposer_t<Pool>(pool);
}

}
