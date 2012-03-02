#pragma once

#include "config.hpp"
#include "fwd.hpp"

#include "error.hpp"
#include "parser.hpp"

namespace calc {

class builder {
public:
    builder()
        : plugin_(0)
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
    CALC_DECL compiler parse(const std::wstring & range, error & err);
    CALC_DECL compiler parse(const std::string & range, error & err);
private:
    parser parser_;
    plugin_compiler plugin_;
};

}
