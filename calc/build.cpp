#include "pch.hpp"

#include "environment.hpp"
#include "exception.hpp"
#include "expression.hpp"

#include "build.hpp"

namespace calc {

compiler parse(const std::wstring & str)
{
    try {
        compiler result;
        parse(str, result);
        return result;
    } catch(build_exception &) {
        throw;
    } catch(boost::exception & e) {
        mstd::rethrow<build_exception>(e);
#if !defined(I3D_OS_S3E)
        std::terminate();
#endif
    } catch(std::exception &) {
        throw build_exception() << mstd::error_message("Unknown exception");
    }
}

program build(const environment & env, const std::wstring & str)
{
    try {
        return parse(str)(boost::bind(&environment::find, &env, _1, _2));
    } catch(build_exception &) {
        throw;
    } catch(undefined_function & exc) {
        throw build_exception() << mstd::error_message("Undefined function: " + exc.name());
    } catch(invalid_arity & exc) {
        throw build_exception() << mstd::error_message((boost::format("Invalid function arity for %s, expected %d arguments, but %d arguments found") 
                                                        % exc.name() % exc.expected() % exc.found()).str());
    } catch(std::exception &) {
        throw build_exception() << mstd::error_message("Unknown exception");
    }
}

}
