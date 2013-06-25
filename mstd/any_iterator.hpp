/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
/  (C) Copyright Sergei Politov 2008. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

#ifndef __ANY_ITERATOR_HPP_
#define __ANY_ITERATOR_HPP_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/utility/enable_if.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/erase.hpp>
#include <boost/mpl/vector.hpp>

#include <boost/tuple/tuple.hpp>

#include <boost/static_assert.hpp>

#include "detail/any_iterator_metafunctions.hpp"
#include "detail/any_iterator_holder.hpp"
#include "detail/any_iterator_forward_feature.hpp"
#include "detail/any_iterator_bidirectional_feature.hpp"
#include "detail/any_iterator_random_access_feature.hpp"

namespace mstd {

template<class Traversal, class Tuple> class get_features_list;

template<class Tuple>
struct get_features_list<boost::forward_traversal_tag, Tuple> {
    typedef boost::mpl::vector<detail::any_iterator_forward_feature<Tuple> > type;
};

template<class Tuple>
struct get_features_list<boost::bidirectional_traversal_tag, Tuple> {
    typedef boost::mpl::vector<detail::any_iterator_forward_feature<Tuple>,
                               detail::any_iterator_bidirectional_feature<Tuple> > type;
};

template<class Tuple>
struct get_features_list<boost::random_access_traversal_tag, Tuple> {
    typedef boost::mpl::vector<detail::any_iterator_forward_feature<Tuple>,
                               detail::any_iterator_bidirectional_feature<Tuple>,
                               detail::any_iterator_random_access_feature<Tuple> > type;
};

template<class Features> class type_item;

template<>
class type_item<boost::mpl::vector0<> > : public virtual detail::any_iterator_holder {
public:
    template<class It> type_item(const It * src) {}
    template<class List> type_item(const type_item<List> & rhs) {}

    void swap(type_item & rhs) {}
};

template<class List>
struct type_item_base {
    typedef type_item<typename boost::mpl::erase<List, typename List::begin>::type > type;
};

template<class List>
class type_item : public List::begin::type, public type_item_base<List>::type {
public:
    typedef typename List::begin::type feature;
    typedef typename type_item_base<List>::type base;

    template<class It>
    type_item(const It * src)
        : feature(src), base(src) {}

    template<class List>
    type_item(const type_item<List> & rhs)
        : feature(rhs), base(rhs) {}

    void swap(type_item & rhs)
    {
        List::begin::type::swap(rhs);
        base::swap(rhs);
    }
};

template<class Traversal, class Tuple> 
class traversal_to_features {
public:
    typedef typename get_features_list<Traversal, Tuple>::type features;
    
    typedef type_item<features> type;
};

template<class Value, class CategoryOrTraversal, class Reference = Value&, class Difference = std::ptrdiff_t>
class any_iterator :
    public boost::iterator_facade<any_iterator<Value, CategoryOrTraversal, Reference, Difference>, 
                                  Value, CategoryOrTraversal, Reference, Difference>,
    private traversal_to_features<typename boost::iterator_category_to_traversal<CategoryOrTraversal>::type,
                                  boost::tuple<Value, Reference, Difference> >::type {

    template<
      class OtherValue,
      class OtherCategoryOrTraversal,
      class OtherReference,
      class OtherDifference
    >
    friend class any_iterator;

public:
    class disabler {};

    typedef typename boost::iterator_category_to_traversal<CategoryOrTraversal>::type Traversal;
    typedef boost::tuple<Value, Reference, Difference> Tuple;

private:
    typedef boost::iterator_facade<any_iterator<Value, CategoryOrTraversal, Reference, Difference>,
                                   Value, CategoryOrTraversal, Reference, Difference> super_type;
                                   
    typedef typename traversal_to_features<Traversal, Tuple>::type features_type;
public:
    any_iterator() {}
    
    any_iterator(const any_iterator & rhs)
        : any_iterator_holder(rhs), super_type(rhs), features_type(rhs) {}
    
    any_iterator& operator=(const any_iterator & rhs)
    {
        if(this != &rhs)
        {
            any_iterator tmp(rhs);
            swap(tmp);
        }

        return *this;
    }
    
    template<class OValue, class OCategoryOrTraversal, class OReference, class ODifference>
    explicit any_iterator(const any_iterator<OValue, OCategoryOrTraversal, OReference, ODifference> & rhs)
        : detail::any_iterator_holder(rhs), features_type(rhs)
    {
        
    }

    template<class WrappedIterator>
    explicit any_iterator(const WrappedIterator & src, 
        typename boost::disable_if<detail::is_any_iterator<WrappedIterator>, disabler>::type = disabler())
        : detail::any_iterator_holder(new detail::any_iterator_holder_concrete_impl<WrappedIterator>(src)),
          features_type(&src)
    {
        BOOST_STATIC_ASSERT((detail::is_iterator_type_erasure_compatible<WrappedIterator, any_iterator>::type::value));
    }

    template<class WrappedIterator>
    typename boost::enable_if<
        boost::mpl::and_<
            detail::is_iterator_type_erasure_compatible<WrappedIterator, any_iterator>,
            boost::mpl::not_<detail::is_any_iterator<WrappedIterator> >
        >,
        any_iterator
        >::type & operator=(WrappedIterator const & src)
    {
        any_iterator tmp(src);
        swap(tmp);

        return *this;
    }
private:
    friend class boost::iterator_core_access;

    any_iterator & swap(any_iterator & other)
    {
        any_iterator_holder::swap(other);
        features_type::swap(other);
        return *this;
    }
};

template<class iterator>
struct make_any_iterator_type
{
    typedef  
        any_iterator<
            typename boost::iterator_value<iterator>::type,
            typename boost::iterator_category<iterator>::type,
            typename boost::iterator_reference<iterator>::type,
            typename boost::iterator_difference<iterator>::type
        > type;
};

}

#endif
