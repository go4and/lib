/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/noncopyable.hpp>

namespace mstd {

template<class Handle, class Traits>
class member_owning_policy {
public:
    explicit member_owning_policy(const Handle & handle)
        : handle_(handle) {}

    // Deprecated
    Handle value() const
    {
        return handle_;
    }

    Handle operator*() const
    {
        return handle_;
    }

    const Handle & handle() const
    {
        return handle_;
    }

    Handle release()
    {
        Handle result = handle_;
        handle_ = Traits::null();
        return result;
    }

    void reset(const Handle & value = Traits::null())
    {
        if(!Traits::is_null(handle()))
            Traits::close(handle());

        handle_ = value;
    }
private:
    Handle handle_;
};

template<class Handle, class Traits>
class inherited_owning_policy : public Handle {
public:
    inherited_owning_policy(const Handle & source)
        : Handle(source) {}

    void reset()
    {
        Traits::close(*this);
    }        
protected:
    const Handle & handle() const
    {
        return *this;
    }
};

template<class Parent>
class comparable_traits : public Parent {
public:
    template<class T>
    static bool is_null(const T & handle) { return handle == Parent::null(); }
};

template<class T>
struct make_null {
    static T null() { return T(); }
};

template<class T>
struct trite_null_provider {
    static const T & null() {
        static const T nV = T();
        return nV;
    }
};

template<class T, class R, R (*Closer)(T), template<class Arg> class NullProvider = trite_null_provider>
struct global_fun_traits : public comparable_traits<NullProvider<T> >{
    static void close(const T & handle) { Closer(handle); }
};

#ifdef BOOST_WINDOWS
template<class T, class R, R (__stdcall *Closer)(T), template<class Arg> class NullProvider = trite_null_provider>
struct global_fun_traits_std : public comparable_traits<NullProvider<T> >{
    static void close(const T & handle) { Closer(handle); }
};
#endif

template<class Handle, class Traits, template<class H, class T> class HandleOwningPolicy = member_owning_policy>
class handle_base : public HandleOwningPolicy<Handle, Traits>, private boost::noncopyable {
private:
    struct dummy {
        void f() {}
    };
    
    typedef void (dummy::*safe_bool)();
public:
    explicit handle_base(const Handle & handle = Traits::null())
        : HandleOwningPolicy<Handle, Traits>(handle) {}

    ~handle_base()
    {
        this->reset();
    }

    bool operator!() const
    {
        return Traits::is_null(this->handle());
    }
    
    operator safe_bool() const
    {
        return Traits::is_null(this->handle()) ? 0 : &dummy::f;
    }
};

}
