#pragma once

#include "singleton.hpp"

namespace mstd {

class any_helper {
public:
    virtual void destroy(void * object) = 0;
    virtual void copy(void * dest, const void * origin) = 0;
    virtual void assign(void * dest, const void * origin) = 0;
    virtual const std::type_info & which() = 0;
};

template<class T>
class any_helper_impl : public any_helper {
    MSTD_SINGLETON_INLINE_DEFINITION(any_helper_impl);
public:
    void destroy(void * object)
    {
        static_cast<T*>(object)->~T();
    }

    void copy(void * dest, const void * origin)
    {
        new (dest) T(*static_cast<const T*>(origin));
    }

    void assign(void * dest, const void * origin)
    {
        T & d = *static_cast<T*>(dest);
        T & o = *const_cast<T*>(static_cast<const T*>(origin));
        d = o;
    }

    virtual const std::type_info & which()
    {
        return typeid(T);
    }
};

template<size_t size, size_t required, bool ok> class any_check_size_helper;

template<size_t size, size_t required> class any_check_size_helper<size, required, true> {};

template<size_t size, size_t required> class any_check_size : public any_check_size_helper<size, required, size >= required> {};

template<size_t size>
class any {
private:
    template<class TT>
    struct clean {
        typedef typename boost::remove_cv<TT>::type type;
    };
public:
    any()
        : helper_(nullptr)
    {
    }

    any(const any & rhs)
        : helper_(rhs.helper_)
    {
        if(helper_)
            helper_->copy(storage_.address(), rhs.storage_.address());
    }

    template<class TT>
    explicit any(const TT & t)
        : helper_(&any_helper_impl<typename clean<TT>::type>::instance())
    {
        BOOST_STATIC_ASSERT(sizeof(any_check_size<size, sizeof(typename clean<TT>::type)>));
        helper_->copy(storage_.address(), &t);
    }

    void operator=(const any & rhs)
    {
        if(helper_ != rhs.helper_)
        {
            reset();
            helper_ = rhs.helper_;
            if(helper_)
                helper_->copy(storage_.address(), rhs.storage_.address());
        } else if(helper_)
            helper_->assign(storage_.address(), rhs.storage_.address());
    }

    template<class TT>
    void operator=(const TT & t)
    {
        typedef typename clean<TT>::type T;
        BOOST_STATIC_ASSERT(sizeof(any_check_size<size, sizeof(T)>));
        any_helper * new_helper = &any_helper_impl<T>::instance();
        if(helper_ != new_helper)
        {
            reset();
            helper_ = new_helper;
            helper_->copy(storage_.address(), &t);
        } else
            helper_->assign(storage_.address(), &t);
    }

    template<class TT>
    typename clean<TT>::type * get()
    {
        typedef typename clean<TT>::type T;
        if(&any_helper_impl<T>::instance() == helper_)
            return static_cast<T*>(storage_.address());
        return nullptr;
    }

    template<class TT>
    const typename clean<TT>::type * get() const
    {
        typedef typename clean<TT>::type T;
        if(&any_helper_impl<T>::instance() == helper_)
            return static_cast<const T*>(storage_.address());
        return nullptr;
    }

    const std::type_info & which()
    {
        return helper_ ? helper_->which() : typeid(void);
    }

    ~any()
    {
        reset();
    }
private:
    void reset()
    {
        if(helper_)
        {
            helper_->destroy(storage_.address());
            helper_ = nullptr;
        }
    }

    any_helper * helper_;
    boost::aligned_storage<size> storage_;
};

template<class T, size_t size>
const T * get(const any<size> * any)
{
    return any->get<T>();
}

template<class T, size_t size>
T * get(any<size> * any)
{
    return any->get<T>();
}

}
