#include "pch.hpp"

#include "environment.hpp"
#include "exception.hpp"
#include "expression.hpp"

#include "build.hpp"

namespace ph = boost::phoenix;
using ph::arg_names::arg1;
using ph::arg_names::arg2;

namespace calc {

compiler parse(const std::wstring & str)
{
    using boost::spirit::labels::_1;
    try {
        compiler result;
        expression expr;
        std::wstring::const_iterator begin = str.begin();
        std::wstring::const_iterator end = str.end();
        bool r = boost::spirit::qi::phrase_parse(begin, end, expr[ph::ref(result) = _1] >> boost::spirit::eoi, boost::spirit::standard_wide::space);
        if(!r || begin != end)
            throw build_exception() << mstd::error_message("Syntax error: " + std::string(begin, begin + std::min(10, end - begin)));
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
        return parse(str)(ph::bind(&environment::find, &env, arg1, arg2));
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
