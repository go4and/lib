#pragma once

#include "config.hpp"
#include "fwd.hpp"

namespace calc {

template<class T>
bool is(const variable & v, const T & t)
{
    const T * val = boost::get<T>(&v);
    return val && *val == t;
}

CALC_DECL bool is_false(const variable & var);
inline bool is_true(const variable & var) { return !is_false(var); }

CALC_DECL std::wstring to_string(const variable & var);
CALC_DECL calc::number to_number(const variable & var);
CALC_DECL const number * as_number(const variable &);
CALC_DECL const std::wstring * as_string(const variable &);

template<class T>
T to_int(const variable & var)
{
    return static_cast<T>(to_number(var));
}

class stack_arg_program : public pre_program {
public:
    explicit stack_arg_program(size_t idx)
        : idx_(idx)
    {
    }

    variable run(void *, variable * stack) const
    {
        return stack[idx_];
    }
private:
    size_t idx_;
};

class stack_arg {
public:
    explicit stack_arg(size_t idx)
        : idx_(idx) {}

    pre_program * operator()(const std::vector<pre_program*>&, const function_lookup&) const
    {
        return new stack_arg_program(idx_);
    }
private:
    size_t idx_;
};

}

