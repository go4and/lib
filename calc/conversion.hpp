/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include "fwd.hpp"
#include "utils.hpp"

namespace calc {

namespace detail {

template<class T>
struct convert;

template<>
struct convert<number> {
public:
    static number apply(const variable & src) { return to_number(src); }
};

template<>
struct convert<std::wstring> {
public:
    static std::wstring apply(const variable & src) { return to_string(src); }
};

template<>
struct convert<variable> {
public:
    static const variable & apply(const variable & src)
    {
        return src;
    }
};

}

}
