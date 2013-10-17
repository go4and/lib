/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>

#include "config.hpp"
#include "mutex.hpp"

namespace mstd {

namespace detail {
    MSTD_DECL void MSTD_STDCALL register_cleaner(void (MSTD_STDCALL *cleaner)());
}

typedef boost::uint32_t once_flag;
#define MSTD_ONCE_INIT 0

void MSTD_STDCALL call_once(once_flag & flag, void (MSTD_STDCALL *f)());

template<class T>
class singleton : private boost::noncopyable {
public:
    static T & instance()
    {
        mstd::call_once(once_, &create_instance);
        return *instance_;
    }
private:
    static void MSTD_STDCALL create_instance()
    {
        instance_ = new T;
        detail::register_cleaner(&singleton<T>::clean);
    }

    static void MSTD_STDCALL clean()
    {
        delete instance_;
    }

    static T* instance_;
    static mstd::mutex mutex_;
    static once_flag once_;
};

template<class T>
T* singleton<T>::instance_;

template<class T>
once_flag singleton<T>::once_ = MSTD_ONCE_INIT;

#define MSTD_SINGLETON_DECLARATION(T) friend class mstd::singleton<T>;
#define MSTD_SINGLETON_DEFINITION(T) public: static T & instance(); private: MSTD_SINGLETON_DECLARATION(T)
#define MSTD_SINGLETON_INLINE_DEFINITION(T) public: static T & instance() { return mstd::singleton<T>::instance(); } private: MSTD_SINGLETON_DECLARATION(T)
#define MSTD_SINGLETON_IMPLEMENTATION(T) T & T::instance() { return mstd::singleton<T>::instance(); }

namespace detail {
    template<class T>
    class default_instance : public singleton<default_instance<T> > {
    public:
        T & value() { return value_; }
    private:
        T value_;
        friend class mstd::singleton<default_instance>;
    };
}

template<class T>
T & default_instance()
{
    return detail::default_instance<T>::instance().value();
}

}
