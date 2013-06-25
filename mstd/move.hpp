/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/type_traits/is_convertible.hpp>

namespace mstd {

template<class T>
struct move_t {
    explicit move_t(T& t_)
        : t(t_) {}

    T& operator*() const
    {
        return t;
    }

    T* operator->() const
    {
        return &t;
    }

    T& t;
private:
    void operator=(move_t&);
};

template<class T>
typename boost::enable_if<boost::is_convertible<T&, move_t<T> >, T>::type move(T& t)
{
    return T(move_t<T>(t));
}

template<class T>
move_t<T> move(move_t<T> t)
{
    return t;
}

}
