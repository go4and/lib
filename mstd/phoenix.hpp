/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <boost/phoenix/core/argument.hpp>

namespace mstd {

template<class ResultType>
struct fixed_result_function {
    template<BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(
                 PHOENIX_LIMIT, typename T, boost::fusion::void_)>
    struct result {
        typedef ResultType type;
    };
};

struct first_func {
    template<BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(
        PHOENIX_LIMIT, typename T, boost::fusion::void_)>
    struct result {
        typedef typename T0::first_type & type;
    };

    template<class Pair>
    typename Pair::first_type & operator()(Pair & pair) const
    {
        return pair.first;
    }
};

const boost::phoenix::function<first_func> first;

struct second_func {
    template<BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(
        PHOENIX_LIMIT, typename T, boost::fusion::void_)>
    struct result {
        typedef typename boost::mpl::if_<boost::is_const<T0>, const typename T0::second_type &, typename T0::second_type &>::type type;
    };

    template<class Pair>
    typename result<Pair>::type operator()(Pair & pair) const
    {
        return pair.second;
    }
};

const boost::phoenix::function<second_func> second;

}
