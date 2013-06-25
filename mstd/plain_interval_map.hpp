/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <vector>

#include <boost/optional.hpp>

namespace mstd {

template<class Key, class Mapped>
struct interval_with_value {
    typedef Key key_type;
    typedef Mapped mapped_type;

    key_type lower;
    key_type upper;
    mapped_type value;

    interval_with_value()
    {
    }
    
    explicit interval_with_value(const key_type & l, const key_type & u, const mapped_type & v)
        : lower(l), upper(u), value(v)
    {
    }
};

struct interval_compare {
    template<class Key, class Mapped>
    bool operator()(const interval_with_value<Key, Mapped> & lhs, const interval_with_value<Key, Mapped> & rhs) const
    {
        return lhs.upper < rhs.upper;
    }

    template<class Key, class Mapped, class Other>
    bool operator()(const interval_with_value<Key, Mapped> & lhs, const Other & rhs) const
    {
        return lhs.upper < rhs;
    }

    template<class Key, class Mapped, class Other>
    bool operator()(const Other & lhs, const interval_with_value<Key, Mapped> & rhs) const
    {
        return lhs < rhs.upper;
    }
};

template<class Key, class Value>
class plain_interval_map {
public:
    typedef Key key_type;
    typedef Value value_type;
    typedef interval_with_value<key_type, value_type> interval_type;
    typedef std::vector<interval_type> holder_type;

    plain_interval_map()
    {
    }

    template<class It>
    plain_interval_map(It begin, It end)
        : intervals_(begin, end)
    {
        std::sort(intervals_.begin(), intervals_.end(), comparator_);
    }

    boost::optional<const value_type &> get(const key_type & k) const
    {
        typename std::vector<interval_type>::const_iterator i = std::lower_bound(intervals_.begin(), intervals_.end(), k, comparator_);
        if(i == intervals_.end() || i->lower > k)
            return boost::optional<const value_type&>();
        return i->value;
    }

    boost::optional<value_type &> get(const key_type & k)
    {
        typename std::vector<interval_type>::const_iterator i = std::lower_bound(intervals_.begin(), intervals_.end(), k, comparator_);
        if(i == intervals_.end() || i->lower > k)
            return boost::optional<value_type&>();
        return i->value;
    }

    const value_type & get(const key_type & k, const value_type & def) const
    {
        boost::optional<const value_type&> temp = get(k);
        return temp ? *temp : def;
    }

    bool empty() const
    {
        return intervals_.empty();
    }

    void swap(std::vector<interval_type> & src)
    {
        intervals_.swap(src);
        std::sort(intervals_.begin(), intervals_.end(), comparator_);
    }
private:
    holder_type intervals_;
    interval_compare comparator_;
};

}
