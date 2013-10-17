/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef MPTREE_BUILDING
#include <vector>

#include <boost/unordered_set.hpp>
#endif

namespace mptree {

bool parse_value(std::string & out, const char * value, size_t len);
bool parse_value(std::wstring & out, const char * value, size_t len);
bool parse_value(bool & out, const char * value, size_t len);
bool parse_value(double & out, const char * value, size_t len);

template<class T>
typename boost::enable_if<boost::is_integral<T>, bool>::type
parse_value(T & out, const char * value, size_t len)
{
    try {
        out = mstd::str2int10_checked<T>(value, len);
        return true;
    } catch(mstd::bad_str2int_cast &) {
        return false;
    }
}

template<class T>
typename boost::enable_if<boost::is_enum<T>, bool>::type
parse_value(T & out, const char * value, size_t len)
{
    return parseEnum(value, len, out);
}

template<class T>
bool parse_value(boost::optional<T> & out, const char * value, size_t len)
{
    if(out)
        return false;
    out = T();
    if(!parse_value(*out, value, len))
    {
        out = boost::optional<T>();
        return false;
    } else
        return true;
}

template<class T>
bool route_parse_value(const char * value, size_t len, void * data)
{
    return parse_value(*static_cast<T*>(data), value, len);
}

class node;

struct parser_state;

typedef parser_state (*child_parser_t)(const char * name, size_t len, void * data);
typedef bool (*parse_value_t)(const char * value, size_t len, void * data);

struct parser_state {
    child_parser_t child_parser;
    parse_value_t parse_value;
    void * data;
    
    parser_state(child_parser_t cp, parse_value_t pv, void * d)
        : child_parser(cp), parse_value(pv), data(d)
    {
    }
};

template<class T>
typename boost::disable_if<boost::is_base_of<node, T>, parser_state>::type
make_parser(T & out)
{
    return parser_state(0, &route_parse_value<T>, &out);
}

template<class T>
parser_state make_parser(std::vector<T> & out)
{
    out.push_back(T());
    return make_parser(out.back());
}

}
