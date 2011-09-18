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

struct new_cloner {
    template<class T>
    T * operator()(const T & t) const
    {
        return new T(t);
    }
};

template<class Key>
struct key_compare {
    template<class T>
    bool operator()(const T & lhs, const Key & rhs) const
    {
        return lhs.key < rhs;
    }

    template<class T>
    bool operator()(const Key & lhs, const T & rhs) const
    {
        return lhs < rhs.key;
    }
};

struct identity {
    template<class T>
    T operator()(const T & t) const
    {
        return t;
    }
};

template<class T, class Key, Key T::*member>
struct key_equals {
    bool operator()(const T & lhs, const Key & rhs) const
    {
        return lhs.*member == rhs;
    }
    
    bool operator()(const Key & lhs, const T & rhs) const
    {
        return lhs == rhs.*member;
    }
    
    bool operator()(const T & lhs, const T & rhs) const
    {
        return lhs.*member == rhs.*member;
    }
};

template<class T>
class cloned_impl {
public:
    explicit cloned_impl(const T & t)
    {
        t_.clone_from(t, mstd::new_cloner(), mstd::delete_disposer());
    }

    cloned_impl(const cloned_impl & rhs)
    {
        t_.clone_from(rhs.t_, mstd::new_cloner(), mstd::delete_disposer());
    }

    ~cloned_impl()
    {
        t_.clear_and_dispose(mstd::delete_disposer());
    }

    operator T&()
    {
        return t_;
    }
private:
    T t_;
};

template<class T>
cloned_impl<T> cloned(const T & t)
{
    return cloned_impl<T>(t);
}

}
