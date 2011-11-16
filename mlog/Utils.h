#pragma once

#if !MLOG_NO_LOGGING

#if !defined(MLOG_BUILDING)
#include <iosfwd>
#include <boost/range/iterator_range.hpp>
#endif

#include "Config.h"

namespace mlog {

class MLOG_DECL OutNow {
public:
    void operator()(std::ostream & out) const;
    void operator()(std::wostream & out) const;
};

inline OutNow onow()
{
    return OutNow();
}

template<class Ch, class CT>
inline std::basic_ostream<Ch, CT> & operator<<(std::basic_ostream<Ch, CT> & out, const OutNow & value)
{
    value(out);
    return out;
}

MLOG_DECL void setup(const char * variable, const char * appname);
MLOG_DECL void setupFromFile(const char * filename, const char * appname);

template<class Collection>
class OutCollection {
public:
    explicit OutCollection(const Collection & collection)
        : collection_(collection) {}

    void operator()(std::ostream & out) const
    {
        out << '{';
        typename Collection::const_iterator i = collection_.begin(), end = collection_.end();
        if(i != end)
        {
            out << *i;
            ++i;
            for(; i != end; ++i)
                out << ", " << *i;
        }
        out << '}';
    }
private:
    const Collection & collection_;
};

template<class Collection>
OutCollection<Collection> ocollection(const Collection & collection)
{
    return OutCollection<Collection>(collection);
}

template<class Iterator>
OutCollection<boost::iterator_range<Iterator> > ocollection(const std::pair<Iterator, Iterator> & p)
{
    return OutCollection<boost::iterator_range<Iterator> >(boost::iterator_range<Iterator>(p.first, p.second));
}

template<class Collection>
inline std::ostream & operator<<(std::ostream & out, const OutCollection<Collection> & value)
{
    value(out);
    return out;
}

template<class Collection>
class OutMap {
public:
    explicit OutMap(const Collection & collection)
        : collection_(collection) {}

    void operator()(std::ostream & out) const
    {
        out << '{';
        typename Collection::const_iterator i = collection_.begin(), end = collection_.end();
        if(i != end)
        {
            out << "[" << i->first << "=>" << i->second << ']';
            ++i;
            for(; i != end; ++i)
                out << ", [" << i->first << "=>" << i->second << ']';
        }
        out << '}';
    }
private:
    const Collection & collection_;
};

template<class Collection>
OutMap<Collection> omap(const Collection & collection)
{
    return OutMap<Collection>(collection);
}

template<class Collection>
inline std::ostream & operator<<(std::ostream & out, const OutMap<Collection> & value)
{
    value(out);
    return out;
}

}

#endif
