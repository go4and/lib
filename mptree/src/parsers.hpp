#pragma once

#ifndef MPTREE_BUILDING
#include <vector>

#include <boost/unordered_set.hpp>
#endif

namespace mptree {

bool parse_value(std::wstring & out, const char * value);
bool parse_value(bool & out, const char * value);
bool parse_value(int & out, const char * value);
bool parse_value(double & out, const char * value);

template<class T>
typename boost::enable_if<boost::is_enum<T>, bool>::type
parse_value(T & out, const char * value)
{
    return parseEnum(value, out);
}

template<class T>
bool parse_value(boost::optional<T> & out, const char * value)
{
    if(out)
        return false;
    out = T();
    if(!parse_value(*out, value))
    {
        out = boost::optional<T>();
        return false;
    } else
        return true;
}

template<class T>
bool route_parse_value(const char * value, void * data)
{
    return parse_value(*static_cast<T*>(data), value);
}

class node;

struct parser_state;

typedef parser_state (*child_parser_t)(const char * name, void * data);
typedef bool (*parse_value_t)(const char * value, void * data);

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
