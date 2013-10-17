/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/fusion/algorithm/query/find_if.hpp>
#include <boost/fusion/iterator/distance.hpp>
#include <boost/fusion/iterator/value_of.hpp>
#include <boost/fusion/sequence/sequence_facade.hpp>
#include <boost/fusion/sequence/intrinsic.hpp>
#include <boost/fusion/support/category_of.hpp>
#include <boost/fusion/view/iterator_range.hpp>

namespace mstd {

    namespace detail {
        template<class Begin, class End>
        struct long_map_storage_gen {
            typedef typename long_map_storage_gen<typename boost::fusion::result_of::next<Begin>::type, End>::type tail;
            typedef boost::fusion::cons<typename boost::fusion::result_of::value_of<Begin>::type::second_type, tail> type;
        };
        
        template<class It>
        struct long_map_storage_gen<It, It> {
            typedef boost::fusion::nil type;
        };
        
        template<class T>
        struct check_first {
            template<class U>
            struct apply : public boost::is_same<typename U::first_type, T> {};
        };
        
        template<class Begin, class End>
        struct check_dupes {
            typedef typename boost::fusion::result_of::deref<Begin>::type begin_type;
            typedef typename boost::fusion::result_of::next<Begin>::type next;
            typedef typename boost::fusion::iterator_range<next, End> next_range;
            typedef typename boost::fusion::result_of::find_if<next_range, check_first<begin_type> >::type dupe_pos;
            enum { value = check_dupes<next, End>::value &&
                       boost::is_same<dupe_pos, End>::value};
        };
        
        template<class It>
        struct check_dupes<It, It> {
            enum { value = 1 };
        };
    }

template<class PairSeq>
struct long_map_defines {
    typedef typename boost::fusion::result_of::begin<PairSeq>::type begin;
    typedef typename boost::fusion::result_of::end<PairSeq>::type end;
    typedef typename detail::long_map_storage_gen<begin, end>::type storage_type;
};

template<class PairSeq>
class long_map : public long_map_defines<PairSeq>,
                 public long_map_defines<PairSeq>::storage_type {
public:
    typedef boost::fusion::sequence_facade_tag fusion_tag;
    struct category : boost::fusion::forward_traversal_tag, boost::fusion::associative_sequence_tag {};
    typedef boost::mpl::false_ is_view;

    BOOST_STATIC_ASSERT((detail::check_dupes<begin, end>::value));
    
    long_map() {}
    
    template<class Seq>
    long_map(const Seq & seq) : storage_type(seq) {}
    
    template<class Seq>
    struct forward_operation {
        typedef typename
            boost::mpl::if_<
                boost::is_const<Seq>
                , typename boost::add_const<storage_type>::type
                , storage_type
            >::type storage;
    };
    
    template<class Seq, class N>
    struct at : public forward_operation<Seq> {
        typedef typename boost::fusion::result_of::at<typename storage, N>::type type;
        
        static type call(Seq & seq)
        {
            return boost::fusion::at<N, typename storage>(seq);
        }
    };

    template<class Seq>
    struct size : public forward_operation<Seq> {
        typedef typename boost::fusion::result_of::size<typename storage>::type type;
        
        static type call(Seq & seq)
        {
            return boost::fusion::size<typename storage>(seq);
        }
    };

    template<class Seq, class Key>
    struct at_key : public forward_operation<Seq> {
        typedef typename boost::fusion::result_of::find_if<PairSeq, detail::check_first<Key> >::type pos;
        typedef boost::fusion::result_of::distance<begin, pos> distance;

        typedef typename boost::fusion::result_of::at<typename storage, distance>::type type;
        
        static type call(Seq & seq)
        {
            return boost::fusion::at<distance, typename storage>(seq);
        }
    };
};

}
