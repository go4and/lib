#pragma once

#ifndef BUILDING_CALC
#include <boost/preprocessor/seq/for_each.hpp>

#include <boost/optional.hpp>
#endif

#include "config.hpp"

namespace calc {

enum error_code {
    error_none,
    error_empty_input,
    error_undefined_function,
    error_invalid_arity,
    error_lexer,
    error_extraneous_input,
    error_missing_token,
    error_mismatched_token,
    error_recognition,
    error_no_viable_alt,
    error_mismatched_set,
    error_early_exit,
    error_syntax,
    error_no_plugin_provided,
};

#define CALC_ERROR_PARAMETERS \
    ((std::wstring, function_name))((size_t, function_arity))((std::string, location))((size_t, expected_arity)) \
    ((std::string, token_value))((bool, token_eof))((uint32_t, token_id))((std::string, message))

class error {
private:
    struct dummy { void nonnull() {} };
    typedef void (dummy::*safe_bool)();
public:
    error()
        : code_(error_none)
    {
    }
    
    error_code code() const
    {
        return code_;
    }

    error & init(error_code code)
    {
        code_ = code;
        return *this;
    }

    void reset()
    {
        code_ = error_none;
    }

    operator safe_bool() const
    {
        return code_ != error_none ? &dummy::nonnull : 0;
    }

    CALC_DECL void out(std::ostream & out) const;

    #define CALC_ERROR_PUBLIC_FUNCTIONS_IMPL(type, name) \
    const type * name() const \
    { \
        return BOOST_PP_CAT(name, _) ? &*BOOST_PP_CAT(name, _) : 0; \
    } \
    error & name(const type & value) \
    { \
        BOOST_PP_CAT(name, _) = value; \
        return *this; \
    } \
    /**/
    #define CALC_ERROR_PUBLIC_FUNCTIONS(r, data, p) CALC_ERROR_PUBLIC_FUNCTIONS_IMPL p
    BOOST_PP_SEQ_FOR_EACH(CALC_ERROR_PUBLIC_FUNCTIONS, ~, CALC_ERROR_PARAMETERS);
private:
    error_code code_;
    #define CALC_ERROR_MEMBERS_IMPL(type, name) boost::optional<type> BOOST_PP_CAT(name, _);
    #define CALC_ERROR_MEMBERS(r, data, p) CALC_ERROR_MEMBERS_IMPL p
    BOOST_PP_SEQ_FOR_EACH(CALC_ERROR_MEMBERS, ~, CALC_ERROR_PARAMETERS);
};

inline bool operator!(const error & err)
{
    return err.code() == error_none;
}

inline std::ostream & operator<<(std::ostream & out, const error & err)
{
    err.out(out);
    return out;
}

class exception : public boost::exception {
};

class calculation_exception : public exception {
};

class division_by_zero_exception : public calculation_exception {
public:
    const char * what() const throw()
    {
        return "division by zero";
    }
};

class comparison_exception : public calculation_exception {
public:
    comparison_exception(const char * op, const std::type_info & lhst, const std::type_info & rhst)
        : op_(op), lhst_(lhst), rhst_(rhst)
    {
    }
    
    const char * op() const
    {
        return op_;
    }
    
    const std::type_info & lhs_type() const
    {
        return lhst_;
    }
    
    const std::type_info & rhs_type() const
    {
        return rhst_;
    }

    const char * what() const throw()
    {
        return "comparison failed";
    }
private:
    const char * op_;
    const std::type_info & lhst_;
    const std::type_info & rhst_;
};

}
