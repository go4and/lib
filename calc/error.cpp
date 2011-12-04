#include "pch.hpp"

#include "error.hpp"

namespace calc {

namespace {

template<class T>
void out_value(std::ostream & out, const T & t)
{
    out << t;
}

void out_value(std::ostream & out, const std::wstring & value)
{
    std::string ustring = mstd::utf8(value);
    out << ustring;
}

}

void error::out(std::ostream & out) const
{
    switch(code_) {
    case error_none:
        out << "no error";
        return;
    case error_empty_input:
        out << "empty input";
        break;
    case error_undefined_function:
        out << "undefined function";
        break;
    case error_invalid_arity:
        out << "invalid arity";
        break;
    case error_lexer:
        out << "lexer error";
        break;
    case error_extraneous_input:
        out << "extraneous input";
        break;
    case error_missing_token:
        out << "missing token";
        break;
    case error_mismatched_token:
        out << "mismatched token";
        break;
    case error_recognition:
        out << "recognition";        
        break;
    case error_no_viable_alt:
        out << "no viable alt";
        break;
    case error_mismatched_set:
        out << "mismatched set";
        break;
    case error_early_exit:
        out << "early exit";
        break;
    case error_syntax:
        out << "syntax";
        break;
    }
    #define CALC_ERROR_OUT_IMPL(type, name) if(BOOST_PP_CAT(name, _)) out_value(out << ", " << BOOST_PP_STRINGIZE(name) << ": ", *BOOST_PP_CAT(name, _));
    #define CALC_ERROR_OUT(r, data, p) CALC_ERROR_OUT_IMPL p
    BOOST_PP_SEQ_FOR_EACH(CALC_ERROR_OUT, ~, CALC_ERROR_PARAMETERS);
}

}
