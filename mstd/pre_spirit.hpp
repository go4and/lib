#pragma once

#include <boost/version.hpp>

#if defined(SPIRIT_VERSION)
#error Spirit already included
#endif

#ifndef BOOST_SPIRIT_THREADSAFE
#define BOOST_SPIRIT_THREADSAFE
#endif

#ifndef PHOENIX_THREADSAFE
#define PHOENIX_THREADSAFE
#endif

#if defined(_DEBUG) && !defined(BOOST_SPIRIT_DEBUG_OUT)
#define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_DEBUG_OUT (::spirit_debug())

#include <fstream>
#include <mstd/strings.hpp>

static std::ostream & spirit_debug()
{
    static std::ofstream out("parse.log");
    return out;
}

namespace std {

inline std::ostream & operator<<(std::ostream & out, const std::wstring & str)
{
    out << mstd::narrow(str);
    return out;
}

#if BOOST_VERSION < 103800
#include <boost/spirit/debug.hpp>
#include <boost/spirit/debug/parser_names.hpp>
#endif

}

#endif
