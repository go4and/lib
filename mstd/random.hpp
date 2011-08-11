#pragma once

#include <limits>

#include <boost/noncopyable.hpp>
#include <boost/random.hpp>

#include "config.hpp"

namespace mstd {

MSTD_DECL boost::mt19937::result_type seed();

template<class Result>
class generator : public boost::noncopyable {
public:
    typedef Result result_type;

    generator(boost::mt19937::result_type seed)
        : rng_(seed),
          dist_(std::numeric_limits<result_type>::min(), std::numeric_limits<result_type>::max()),
          impl_(&rng_, dist_) {}

    generator()
        : rng_(seed()),
          dist_(std::numeric_limits<result_type>::min(), std::numeric_limits<result_type>::max()),
          impl_(&rng_, dist_) {}

    result_type operator()()
    {
        return impl_();
    }
private:
    typedef boost::mt19937 generator_type;
    typedef boost::uniform_int<result_type> distribution_type;
    typedef boost::variate_generator<boost::mt19937*, boost::uniform_int<result_type> > implementation_type;

    generator_type rng_;
    distribution_type dist_;
    implementation_type impl_;
};

}
