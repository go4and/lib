#pragma once

#include <boost/scoped_ptr.hpp>

#include "config.hpp"

#include "singleton.hpp"

namespace mstd {

class MSTD_DECL token_generator_base {
public:
    std::string next(size_t len);

    token_generator_base(const char * source);
    token_generator_base(const std::string & source);

    ~token_generator_base();
private:
    class Impl;
    boost::scoped_ptr<Impl> impl_;
};

class MSTD_DECL token_generator : public mstd::singleton<token_generator>, public token_generator_base {
private:
    token_generator();
    
    MSTD_SINGLETON_DECLARATION(token_generator);
};

}
