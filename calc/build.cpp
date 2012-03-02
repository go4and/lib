#include "pch.hpp"

#include "environment.hpp"
#include "error.hpp"

#include "build.hpp"

namespace calc {

compiler builder::parse(const std::wstring & str, error & err)
{
    compiler result;
    parser_.parse(str, result, err);
    return result;
}

compiler builder::parse(const std::string & str, error & err)
{
    compiler result;
    parser_.parse(str, result, err);
    return result;
}

program builder::build(const environment & env, const std::wstring & str, error & err, const plugin_compiler * plugin)
{
    compiler c = parse(str, err);
    if(err)
        return program();
    compiler_context context = { boost::bind(&environment::find, &env, _1, _2), err, plugin };
    return c(context);
}

program builder::build(const environment & env, const std::string & str, error & err, const plugin_compiler * plugin)
{
    compiler c = parse(str, err);
    if(err)
        return program();
    compiler_context context = { boost::bind(&environment::find, &env, _1, _2), err, plugin };
    return c(context);
}

}
