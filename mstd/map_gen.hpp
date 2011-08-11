#pragma once

#include <boost/mpl/apply.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/reverse_fold.hpp>

#include <boost/fusion/container/list/cons.hpp>
#include <boost/fusion/support/pair.hpp>

#include "mstd/long_map.hpp"

namespace mstd {

template<class Seq, class Functor>
class map_gen {
public:
    template<class Tail, class Cur>
    struct make_cons {
        typedef boost::fusion::cons<
                    boost::fusion::pair<
                        Cur, typename boost::mpl::apply<typename boost::mpl::lambda<Functor>::type, Cur>::type
                    >,
                    Tail> type;
    };

    typedef typename 
        boost::mpl::reverse_fold<
            Seq, 
            boost::fusion::nil, 
            make_cons<
                boost::mpl::placeholders::_1, boost::mpl::placeholders::_2
            >
        >::type pairs;
    typedef typename long_map<pairs> type;
};

}
