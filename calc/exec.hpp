#pragma once

#ifndef BUILDING_CALC
#include <mstd/exception.hpp>
#endif

#include "fwd.hpp"

namespace calc {

class Exec;
typedef mstd::own_exception<Exec> exec_exception;

program wrap(const program & ops);

template<class E>
class WrapperEx {
public:
    explicit WrapperEx(const program & p)
        : impl_(p) {}

    variable operator()(void * ctx, variable * stack) const
    {
        try {
            return impl_(ctx, stack);
        } catch(exec_exception & exc) {
            mstd::rethrow<E>(exc);
            return *static_cast<variable*>(0);
        }
    }
private:
    program impl_;
};

template<class Exception>
program wrapEx(const program & p)
{
    return WrapperEx<Exception>(p);
}

}
