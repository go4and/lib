#pragma once

#include "config.hpp"
#include "fwd.hpp"

namespace calc {

class Build;
typedef mstd::own_exception<Build> build_exception;

CALC_DECL program build(const environment & env, const std::wstring & range);

template<class Exception>
program buildEx(const environment & env, const std::wstring & range)
{
    try {
        return build(env, range);
    } catch(build_exception & exc) {
        mstd::rethrow<Exception>(exc);
#if !defined(I3D_OS_S3E)
        terminate();
#endif
    }
}

CALC_DECL compiler parse(const std::wstring & range);

}
