#pragma once

#include "fwd.hpp"

typedef struct ANTLR3_INPUT_STREAM_struct * pANTLR3_INPUT_STREAM;
typedef struct calc_exprLexer_Ctx_struct * pcalc_exprLexer;
typedef	struct ANTLR3_COMMON_TOKEN_STREAM_struct * pANTLR3_COMMON_TOKEN_STREAM;
typedef struct calc_exprParser_Ctx_struct * pcalc_exprParser;
struct ANTLR3_BASE_RECOGNIZER_struct;

namespace calc {

class parser : public boost::noncopyable {
public:
    parser()
        : input_(0)
    {
    }

    CALC_DECL void parse(const std::wstring & inp, compiler & result);

    CALC_DECL ~parser();
private:
    pANTLR3_INPUT_STREAM input_;
    pcalc_exprLexer lex_;
    pANTLR3_COMMON_TOKEN_STREAM tokens_;
    pcalc_exprParser parser_;
    std::vector<char> buffer_;
};

}
