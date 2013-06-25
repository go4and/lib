/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include "strings.hpp"

namespace mstd {

template<class T>
class wide_stream_impl {
public:
    explicit wide_stream_impl(T & impl)
        : impl_(impl) {}

    T & impl()
    {
        return impl_;
    }
private:
    T & impl_;
};

template<class T>
wide_stream_impl<T> wide_stream(T & t)
{
    return wide_stream_impl<T>(t);
}

template<class T, class Value>
wide_stream_impl<T> & operator<<(wide_stream_impl<T> & stream, const Value & value)
{
    stream.impl() << value;
    return stream;
}

template<class T>
wide_stream_impl<T> & operator<<(wide_stream_impl<T> & stream, const std::string & value)
{
    stream.impl() << widen(value);
    return stream;
}

template<class T>
wide_stream_impl<T> & operator<<(wide_stream_impl<T> & stream, std::wostream & func(std::wostream&))
{
    stream.impl() << func;
    return stream;
}

}
