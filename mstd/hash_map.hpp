/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

namespace mstd {

template<class Key, class Value>
struct hash_map {
    typedef std::pair<Key, Value> value_type;
    typedef boost::multi_index::multi_index_container<
            value_type,
            boost::multi_index::indexed_by <
                boost::multi_index::hashed_unique<
                    boost::multi_index::member<value_type, const Key, &value_type::first>
                >
            >
        > type;
};

template<class Key, class Value>
struct hash_multimap {
    typedef std::pair<Key, Value> value_type;
    typedef boost::multi_index::multi_index_container<
            value_type,
            boost::multi_index::indexed_by <
                boost::multi_index::hashed_non_unique<
                    boost::multi_index::member<value_type, const Key, &value_type::first>
                >
            >
        > type;
};

}
