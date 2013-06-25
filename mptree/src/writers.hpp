/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#ifndef MPTREE_BUILDING
#include <mstd/itoa.hpp>
#endif

namespace mptree {

class node_writer {
public:
    virtual void begin_struct(const char * name, bool in_array) = 0;
    virtual void end_struct(const char * name, bool in_array) = 0;

    virtual void begin_value(const char * name, bool in_array) = 0;
    virtual void end_value(const char * name, bool in_array) = 0;

    virtual void begin_array(const char * name) = 0;
    virtual void end_array(const char * name) = 0;

    virtual void empty_node(const char * name) = 0;

    virtual void write_raw(const char * buffer, size_t len) = 0;
    virtual void write_escaped(const wchar_t * value, size_t len) = 0;
    virtual void write_escaped(const char * value, size_t len) = 0;
};

class node;

inline void write_value(node_writer & writer, const std::wstring & value) { writer.write_escaped(value.c_str(), value.length()); }
inline void write_value(node_writer & writer, const std::string & value) { writer.write_escaped(value.c_str(), value.length()); }

char render_short_value(...);

inline size_t render_short_value(char * out, bool value)
{
    if(value)
    {
        memcpy(out, "true", 4);
        return 4;
    } else {
        memcpy(out, "false", 5);
        return 5;
    }
}

size_t render_short_value(char * out, double value);

template<class T>
typename boost::enable_if<boost::is_integral<T>, size_t>::type render_short_value(char * out, const T & value)
{
    mstd::itoa(value, out);
    return strlen(out);
}

template<class T>
typename boost::enable_if<boost::is_enum<T>, size_t>::type render_short_value(char * out, const T & value)
{
    const char * temp = name(value);
    size_t result = strlen(temp);
    memcpy(out, temp, result);
    return result;
}

void write_value(std::streambuf * buf, double value);

template<class T>
inline typename boost::disable_if_c<sizeof(render_short_value(0, T())) == 1, void>::type
write_value(node_writer & writer, const T & t)
{
    char buffer[0x40];
    writer.write_raw(buffer, render_short_value(buffer, t));
}

template<class T>
inline typename boost::disable_if<boost::is_base_of<node, T>, void>::type
write_node(node_writer & writer, const T & t, const char * name, bool in_array)
{
    writer.begin_value(name, in_array);
    write_value(writer, t);
    writer.end_value(name, in_array);
}

template<class T>
void write_node(node_writer & writer, const boost::optional<T> & value, const char * name, bool in_array)
{
    if(value)
        write_node(writer, *value, name, in_array);
}

template<class T>
void write_node(node_writer & writer, const std::vector<T> & v, const char * name, bool in_array)
{
    BOOST_ASSERT(!in_array);
    writer.begin_array(name);
    for(auto i = v.begin(), end = v.end(); i != end; ++i)
        write_node(writer, *i, name, true);
    writer.end_array(name);
}

}
