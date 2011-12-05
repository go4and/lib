#pragma once

#include "config.hpp"
#include "fwd.hpp"

#include "error.hpp"
#include "parser.hpp"

namespace calc {

class builder {
public:
    CALC_DECL program build(const environment & env, const std::wstring & range, error & err);
    CALC_DECL program build(const environment & env, const std::string & range, error & err);
    CALC_DECL compiler parse(const std::wstring & range, error & err);
    CALC_DECL compiler parse(const std::string & range, error & err);
private:
    parser parser_;
};

}
