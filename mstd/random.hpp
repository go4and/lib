/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <limits>

#include <boost/noncopyable.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "config.hpp"

namespace mstd {

MSTD_DECL boost::mt19937::result_type seed();

template<class Result>
class generator : public boost::noncopyable {
public:
    typedef Result result_type;

    generator(boost::mt19937::result_type seed)
        : rng_(seed),
          dist_(std::numeric_limits<result_type>::min(), std::numeric_limits<result_type>::max())
    {}

    generator()
        : rng_(seed()),
          dist_(std::numeric_limits<result_type>::min(), std::numeric_limits<result_type>::max())
    {}

    result_type operator()()
    {
        return dist_(rng_);
    }
private:
    typedef boost::mt19937 generator_type;
    typedef boost::random::uniform_int_distribution<result_type> distribution_type;

    generator_type rng_;
    distribution_type dist_;
};

template<class Generator>
void generate_bytes(char * out, size_t len, Generator & generator)
{
    for(;;)
    {
        auto temp = generator();
        if(len > sizeof(temp))
        {
            memcpy(out, &temp, sizeof(temp));
            out += sizeof(temp);
            len -= sizeof(temp);
        } else {
            memcpy(out, &temp, len);
            break;
        }
    }
}

}
