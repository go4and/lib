/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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

program builder::build(const environment & env, const std::wstring & str, error & err)
{
    return build([&env](const std::wstring & name, int arity, bool la, bool lowered){ return env.find(name, arity, lowered); }, str, err);
}

program builder::build(const environment & env, const std::string & str, error & err)
{
    return build([&env](const std::wstring & name, int arity, bool la, bool lowered){ return env.find(name, arity, lowered); }, str, err);
}

program builder::build(const function_lookup & lookup, const std::wstring & str, error & err)
{
    compiler c = parse(str, err);
    if(err)
        return program();
    compiler_context context = { lookup, err, plugin_ };
    return c(context);
}

program builder::build(const function_lookup & lookup, const std::string & str, error & err)
{
    compiler c = parse(str, err);
    if(err)
        return program();
    compiler_context context = { lookup, err, plugin_ };
    return c(context);
}

}
