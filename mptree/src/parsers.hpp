#pragma once

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

class node;

template<class T, class Node>
typename boost::disable_if<boost::is_base_of<node, T>, bool>::type
parse_node(T & out, const Node & node)
{
    return parse_value(out, node_value(node));
}

template<class T, class Node>
bool parse_node(std::vector<T> & out, const Node & node)
{
    out.push_back(T());
    if(!parse_node(out.back(), node))
    {
        out.pop_back();
        return false;
    } else
        return true;
}

}
