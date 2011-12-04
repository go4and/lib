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

program builder::build(const environment & env, const std::wstring & str, error & err)
{
    compiler c = parse(str, err);
    if(err)
        return program();
    return c(boost::bind(&environment::find, &env, _1, _2), err);
}

}
