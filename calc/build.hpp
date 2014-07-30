/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include "config.hpp"
#include "fwd.hpp"

#include "error.hpp"
#include "parser.hpp"

namespace calc {

class builder {
public:
    builder()
    {
    }

    void plugin(const plugin_compiler & value)
    {
        plugin_ = value;
    }

    const plugin_compiler & plugin() const
    {
        return plugin_;
    }

    CALC_DECL program build(const environment & env, const std::wstring & range, error & err);
    CALC_DECL program build(const environment & env, const std::string & range, error & err);
    CALC_DECL program build(const function_lookup & lookup, const std::wstring & str, error & err);
    CALC_DECL program build(const function_lookup & lookup, const std::string & str, error & err);
    CALC_DECL compiler parse(const std::wstring & range, error & err);
    CALC_DECL compiler parse(const std::string & range, error & err);
private:
    parser parser_;
    plugin_compiler plugin_;
};

}
