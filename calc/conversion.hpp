#pragma once

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
