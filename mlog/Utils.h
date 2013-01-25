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

MLOG_DECL bool setup(const char * variable, const char * appname);
MLOG_DECL bool setupFromFile(const char * filename, const char * appname);

template<class T1, class T2>
std::ostream & operator<<(std::ostream & out, const std::pair<T1, T2> & p)
{
    return out << '(' << p.first << ',' << p.second << ')';
}

template<class Collection>
class OutCollection {
public:
    explicit OutCollection(Collection collection)
        : collection_(collection) {}

    void operator()(std::ostream & out) const
    {
        out << '{';
        output(out, collection_.begin(), collection_.end());
        out << '}';
    }
    
private:
    template<class It>
    static inline void output(std::ostream & out, It i, It end)
    {
        if(i != end)
        {
            out << *i;
            ++i;
            for(; i != end; ++i)
                out << ", " << *i;
        }
    }

    Collection collection_;
};

template<class Collection>
OutCollection<const Collection&> ocollection(const Collection & collection)
{
    return OutCollection<const Collection &>(collection);
}

template<class Iterator>
OutCollection<boost::iterator_range<Iterator> > ocollection(const std::pair<Iterator, Iterator> & p)
{
    return OutCollection<boost::iterator_range<Iterator> >(boost::iterator_range<Iterator>(p.first, p.second));
}

template<class Iterator>
OutCollection<boost::iterator_range<Iterator> > ocollection(Iterator begin, Iterator end)
{
    return OutCollection<boost::iterator_range<Iterator> >(boost::iterator_range<Iterator>(begin, end));
}

template<class Iterator>
OutCollection<boost::iterator_range<Iterator> > ocollection(Iterator begin, size_t len)
{
    Iterator end = begin;
    std::advance(end, len);
    return OutCollection<boost::iterator_range<Iterator> >(boost::iterator_range<Iterator>(begin, end));
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
