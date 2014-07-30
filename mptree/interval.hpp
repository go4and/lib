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
#include <boost/array.hpp>

#include <boost/algorithm/string/trim.hpp>
#endif

#include "src/writers.hpp"

namespace mptree {

template<class T>
struct interval {
    typedef T value_type;

    value_type first;
    value_type last;
    
    interval() {}
    
    interval(value_type f, value_type l)
        : first(f), last(l)
    {
    }
};

template<class T>
std::ostream & operator<<(std::ostream & out, const interval<T> & i)
{
    if(i.first == i.last)
        out << i.first;
    else
        out << i.first << '-' << i.last;
    return out;
}

template<class T, class It>
T parseInt(It begin, It end)
{
    auto p = boost::trim_copy(boost::make_iterator_range(begin, end));
    begin = p.begin(); end = p.end();
    if(begin != end && *begin == '0')
    {
        auto temp = begin;
        ++temp;
        if(temp != end && *temp == 'x')
            return mstd::str2int16_checked<T>(++temp, end);
    }
    return mstd::str2int10_checked<T>(begin, end);
}

template<class T, class Ch>
bool parse_value(interval<T> & out, const Ch * val, size_t len)
{
    const Ch * end = val + len;
    const Ch * p = std::find(val, end, '-');
    try {
        if(p != end)
        {
            int first = parseInt<T>(val, p);
            ++p;
            out.last = parseInt<T>(p, end);
            out.first = first;
        } else {
            out.first = out.last = parseInt<T>(val, end);
        }
        return true;
    } catch(mstd::bad_str2int_cast&) {
        return false;
    }
}

template<class T, class Collection = std::vector<interval<T> > >
struct intervals {
    typedef interval<T> value_type;
    typedef Collection collection_type;
    typedef typename collection_type::iterator iterator;
    typedef typename collection_type::const_iterator const_iterator;
    collection_type value;

    void add(int f, int l)
    {
        value.push_back(value_type(f, l));
    }

    inline const_iterator begin() const { return value.begin(); }
    inline const_iterator end() const { return value.end(); }
    inline iterator begin() { return value.begin(); }
    inline iterator end() { return value.end(); }
};

template<class T, class Col>
std::ostream & operator<<(std::ostream & out, const intervals<T, Col> & c)
{
    out << '[';
    auto begin = c.begin();
    auto end = c.end();
    if(begin != end)
    {
        out << *begin;
        while(++begin != end)
        {
            out << ", ";
            out << *begin;
        }
    }
    return out << ']';
}

template<class T>
char * render_interval(const interval<T> & interval, char * buffer)
{
    mstd::itoa(interval.first, buffer);
    char * stop = buffer + strlen(buffer);
    if(interval.first != interval.last)
    {
        *stop++ = '-';
        mstd::itoa(interval.last, stop);
        stop += strlen(stop);
    }
    return stop;
}

template<class T>
void write_value(node_writer & writer, const interval<T> & interval)
{
    char buffer[0x20];
    char * stop = render_interval(interval, buffer);
    writer.write_raw(buffer, stop - buffer);
}

template<class T, class It>
bool parse_value(intervals<T> & out, It val, const It & end)
{
    boost::array<interval<T>, 0x10> buffer;
    std::vector<interval<T> > temp;
    size_t n = 0;
    try {
        interval<T> interval;
        while(val != end)
        {
            auto q = std::find(val, end, ',');
            auto p = std::find(val, q, '-');
            if(p != q)
            {
                interval.first = parseInt<T>(val, p);
                ++p;
                interval.last = parseInt<T>(p, q);
            } else {
                interval.first = interval.last = parseInt<T>(val, q);
            }
            if(n < buffer.size())
                buffer[n++] = interval;
            else
                temp.push_back(interval);
            val = q;
            if(val != end)
                ++val;
            else
                break;
        }
        out.value.reserve(n + temp.size());
        out.value.assign(buffer.begin(), buffer.begin() + n);
        if(n >= buffer.size())
            out.value.insert(out.value.end(), temp.begin(), temp.end());
        return true;
    } catch(mstd::bad_str2int_cast&) {
        return false;
    }
}

template<class T, class It>
bool parse_value(intervals<T> & out, const It & val, size_t len)
{
    return parse_value(out, val, val + len);
}

template<class T>
void write_value(node_writer & writer, const intervals<T> & intervals)
{
    char buffer[0x21];
    for(auto i = intervals.begin(), end = intervals.end(); i != end;)
    {
        char * stop = render_interval(*i, buffer);
        if(++i != end)
            *stop++ = ',';
        writer.write_raw(buffer, stop - buffer);
    }
}

}
