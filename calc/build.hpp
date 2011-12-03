#pragma once

#include "config.hpp"
#include "fwd.hpp"

#include "expression.hpp"

namespace calc {

class builder;
typedef mstd::own_exception<builder> build_exception;

class builder {
public:
    CALC_DECL program build(const environment & env, const std::wstring & range);

    template<class Exception>
    program buildEx(const environment & env, const std::wstring & range)
    {
        try {
            return build(env, range);
        } catch(build_exception & exc) {
            mstd::rethrow<Exception>(exc);
    #if !defined(I3D_OS_S3E)
            std::terminate();
    #endif
        }
    }

    CALC_DECL compiler parse(const std::wstring & range);
private:
    parser parser_;
};

}
