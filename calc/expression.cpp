#include "pch.hpp"

#include "fwd.hpp"
#include "environment.hpp"
#include "exception.hpp"

#include "calc_exprLexer.h"
#include "calc_exprParser.h"

#include "expression.hpp"

namespace calc {

namespace {

pANTLR3_BASE_TREE childAt(pANTLR3_BASE_TREE tree, ANTLR3_UINT32 index)
{
    return (pANTLR3_BASE_TREE) tree->children->get(tree->children, index);
}

std::wstring getText(pANTLR3_BASE_TREE tree)
{
    return mstd::deutf8(mstd::pointer_cast<const char*>(tree->getText(tree)->chars));
}

void makeCompiler(pANTLR3_BASE_TREE tree, compiler & out);

template<class Op, class Result>
struct UnaryProgram {
    Op op;
    program lhs;
    
    explicit UnaryProgram(Op o, const program & l)
        : op(o), lhs(l)
    {
    }
    
    variable operator()(void * ctx, variable * stack) const
    {
        return op(detail::convert<Result>::apply(lhs(ctx, stack)));
    }
};

template<class Op, class Result>
struct UnaryCompiler {
    Op op;
    compiler lhs;

    explicit UnaryCompiler(Op o)
        : op(o)
    {
    }
    
    program operator()(const function_lookup & lookup) const
    {
        return UnaryProgram<Op, Result>(op, lhs(lookup));
    }
};

template<class Result, class Op>
void makeUnaryCompiler(pANTLR3_BASE_TREE tree, Op op, compiler & out)
{
    BOOST_ASSERT(tree->getChildCount(tree) == 1);
    UnaryCompiler<Op, Result> result(op);
    makeCompiler(childAt(tree, 0), result.lhs);
    out = result;
}

struct AndProgram {
    program lhs;
    program rhs;

    explicit AndProgram(const program & l, const program & r)
        : lhs(l), rhs(r)
    {
    }

    variable operator()(void * ctx, variable * stack) const
    {
        variable v = lhs(ctx, stack);
        return is_true(v) ? rhs(ctx, stack) : v;
    }
};

struct OrProgram {
    program lhs;
    program rhs;

    explicit OrProgram(const program & l, const program & r)
        : lhs(l), rhs(r)
    {
    }

    variable operator()(void * ctx, variable * stack) const
    {
        variable v = lhs(ctx, stack);
        return is_true(v) ? v : rhs(ctx, stack);
    }
};

template<class Op, class Argument>
struct BinaryProgram {
    Op op;
    program lhs;
    program rhs;
    
    explicit BinaryProgram(const program & l, const program & r)
        : lhs(l), rhs(r)
    {
    }
    
    variable operator()(void * ctx, variable * stack) const
    {
        return op(detail::convert<Argument>::apply(lhs(ctx, stack)),
                  detail::convert<Argument>::apply(rhs(ctx, stack)));
    }
};

template<class Program>
struct BinaryCompiler {
    compiler lhs;
    compiler rhs;

    program operator()(const function_lookup & lookup) const
    {
        return Program(lhs(lookup), rhs(lookup));
    }
};

template<class Program>
void makeBinaryCompiler(pANTLR3_BASE_TREE tree, compiler & out)
{
    BOOST_ASSERT(tree->getChildCount(tree) == 2);
    BinaryCompiler<Program> result;
    makeCompiler(childAt(tree, 0), result.lhs);
    makeCompiler(childAt(tree, 1), result.rhs);
    out = result;
}

template<class Value>
struct ValueCompiler {
    Value value;

    explicit ValueCompiler(const Value & v)
        : value(v)
    {
    }
    
    program operator()(const function_lookup & lookup) const
    {
        return *this;
    }
    
    variable operator()(void * ctx, variable * stack) const
    {
        return value;
    }
};

template<class Value>
void makeValueCompiler(const Value & value, compiler & out)
{
    out = ValueCompiler<Value>(value);
}

struct InvokationCompiler {
    std::wstring name;
    std::vector<compiler> args;

    program operator()(const function_lookup & lookup) const
    {
        func d = lookup(name, args.size());
        if(d.function.empty())
            throw undefined_function(mstd::utf8(name));
        if(static_cast<size_t>(d.arity) != args.size())
            throw invalid_arity(mstd::utf8(name), d.arity, args.size());
        std::vector<program> p;
        p.reserve(args.size());
        for(std::vector<compiler>::const_iterator i = args.begin(), end = args.end(); i != end; ++i)
            p.push_back((*i)(lookup));
        return d.function(p, lookup);
    }
};

void makeInvokation(pANTLR3_BASE_TREE tree, compiler & out)
{
    InvokationCompiler result;
    result.name = getText(tree);
    size_t count = tree->getChildCount(tree);
    result.args.resize(count);
    for(size_t i = 0; i != count; ++i)
        makeCompiler(childAt(tree, i), result.args[i]);
    out = result;
}

struct mod {
    number operator()(number a1, number a2) const
    {
        return a2 != 0 ? a1 % a2 : a1;
    }
};

struct div {
    number operator()(number a1, number a2) const
    {
        return a2 != 0 ? a1 / a2 : a1;
    }
};

template<class T>
struct or_ {
    T operator()(const T & t1, const T & t2) const
    {
        return t1 | t2;
    }
};

template<class T>
struct xor_ {
    T operator()(const T & t1, const T & t2) const
    {
        return t1 ^ t2;
    }
};

template<class T>
struct and_ {
    T operator()(const T & t1, const T & t2) const
    {
        return t1 & t2;
    }
};

template<class T>
struct shift_right {
    T operator()(const T & t1, const T & t2) const
    {
        return t1 >> t2;
    }
};

template<class T>
struct shift_left {
    T operator()(const T & t1, const T & t2) const
    {
        return t1 << t2;
    }
};

struct size_ {
    number operator()(const std::wstring & inp) const
    {
        return inp.size();
    }
};

struct ne_ {
    bool operator()(const variable & t1, const variable & t2) const
    {
        return !(t1 == t2);
    }
};

struct greater_ {
    bool operator()(const variable & t1, const variable & t2) const
    {
        return t2 < t1;
    }
};

struct ge_ {
    bool operator()(const variable & t1, const variable & t2) const
    {
        return !(t1 < t2);
    }
};

struct le_ {
    bool operator()(const variable & t1, const variable & t2) const
    {
        return !(t2 < t1);
    }
};

std::wstring parseString(pANTLR3_UINT8 inp)
{
    const char * temp = mstd::pointer_cast<const char*>(inp);
    std::wstring result = mstd::deutf8(temp + 1, strlen(temp) - 2);
    std::wstring::iterator w = result.begin(), end = result.end();
    for(std::wstring::iterator i = w; i != end; ++i)
    {
        if(*i == L'\\')
        {
            *w++ = *++i;
        } else {
            if(i != w)
                *w = *i;
            ++w;
        }
    }
    result.erase(w, end);
    return result;
}

void makeCompiler(pANTLR3_BASE_TREE tree, compiler & out)
{
    pANTLR3_COMMON_TOKEN token = tree->getToken(tree);
    BOOST_ASSERT(token);
    switch(token->type) {
    case T_AND:
        makeBinaryCompiler<AndProgram>(tree, out);
        break;
    case T_ASTRING:
        makeValueCompiler(parseString(tree->getText(tree)->chars), out);
        break;
    case T_BITOR:
        makeBinaryCompiler<BinaryProgram<or_<number>, number> >(tree, out);
        break;
    case T_BITXOR:
        makeBinaryCompiler<BinaryProgram<xor_<number>, number> >(tree, out);
        break;
    case T_BITAND:
        makeBinaryCompiler<BinaryProgram<and_<number>, number> >(tree, out);
        break;
    case T_CONCAT:
        makeBinaryCompiler<BinaryProgram<std::plus<std::wstring>, std::wstring> >(tree, out);
        break;
    case T_DIV:
        makeBinaryCompiler<BinaryProgram<div, number> >(tree, out);
        break;
    case T_EQ:
        makeBinaryCompiler<BinaryProgram<std::equal_to<variable>, variable> >(tree, out);
        break;
    case T_GE:
        makeBinaryCompiler<BinaryProgram<ge_, variable> >(tree,  out);
        break;
    case T_GREATER:
        makeBinaryCompiler<BinaryProgram<greater_, variable> >(tree, out);
        break;
    case T_HASH:
        makeUnaryCompiler<std::wstring>(tree, size_(), out);
        break;
    case T_HEX_NUMBER:
        makeValueCompiler(mstd::str2int16<number>(mstd::pointer_cast<const char*>(tree->getText(tree)->chars) + 2), out);
        break;
    case T_IDENTIFIER:
        makeInvokation(tree, out);
        break;
    case T_LE:
        makeBinaryCompiler<BinaryProgram<le_, variable> >(tree, out);
        break;
    case T_LESS:
        makeBinaryCompiler<BinaryProgram<std::less<variable>, variable> >(tree, out);
        break;
    case T_MINUS:
        if(tree->getChildCount(tree) == 1)
            makeUnaryCompiler<number>(tree, std::negate<number>(), out);
        else
            makeBinaryCompiler<BinaryProgram<std::minus<number>, number> >(tree, out);
        break;
    case T_MOD:
        makeBinaryCompiler<BinaryProgram<mod, number> >(tree, out);
        break;
    case T_MUL:
        makeBinaryCompiler<BinaryProgram<std::multiplies<number>, number> >(tree, out);
        break;
    case T_NE:
        makeBinaryCompiler<BinaryProgram<ne_, variable> >(tree, out);
        break;
    case T_NOT:
        makeUnaryCompiler<number>(tree, std::logical_not<number>(), out);
        break;
    case T_NUMBER:
        makeValueCompiler(mstd::str2int10<number>(mstd::pointer_cast<const char*>(tree->getText(tree)->chars)), out);
        break;
    case T_QSTRING:
        makeValueCompiler(parseString(tree->getText(tree)->chars), out);
        break;
    case T_OR:
        makeBinaryCompiler<OrProgram>(tree, out);
        break;
    case T_PLUS:
        makeBinaryCompiler<BinaryProgram<std::plus<number>, number> >(tree, out);
        break;
    case T_SHIFT_LEFT:
        makeBinaryCompiler<BinaryProgram<shift_left<number>, number> >(tree, out);
        break;
    case T_SHIFT_RIGHT:
        makeBinaryCompiler<BinaryProgram<shift_right<number>, number> >(tree, out);
        break;
    default:
        std::cout << "unknown token: " << token->type << std::endl;
        BOOST_ASSERT(false);
        break;
    }
}

void outTree(int level,pANTLR3_BASE_TREE pTree)
{
    ANTLR3_UINT32 childcount =  pTree->getChildCount(pTree);

    for (ANTLR3_UINT32 i=0;i<childcount;i++)
    {
        pANTLR3_BASE_TREE pChild = (pANTLR3_BASE_TREE) pTree->children->get(pTree->children,i);
        for (int j=0;j<level;j++)
        {
            std::cout << "  ";
        }
        pANTLR3_COMMON_TOKEN token = pChild->getToken(pChild);
        if(token)
            std::cout << '[' << token->type << ']';
        std::cout << 
            pChild->getText(pChild)->chars <<       
            std::endl;
        int f=pChild->getChildCount(pChild);
        if (f>0)
        {
            outTree(level+1,pChild);
        }
    }
}

}

void parser::parse(const std::wstring & inp, compiler & result)
{
    std::string utf8inp = mstd::utf8(inp);
    pANTLR3_INPUT_STREAM input;
    if(input_)
    {
        input = static_cast<pANTLR3_INPUT_STREAM>(input_);
        input->reuse(input, pANTLR3_UINT8(utf8inp.c_str()), utf8inp.length(), pANTLR3_UINT8("expression"));
    } else {
        input = antlr3StringStreamNew(pANTLR3_UINT8(utf8inp.c_str()), ANTLR3_ENC_UTF8, utf8inp.length(), pANTLR3_UINT8("expression"));
        input_ = input;
    }
    pcalc_exprLexer lex = calc_exprLexerNew(input);
    pANTLR3_COMMON_TOKEN_STREAM tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lex));
    pcalc_exprParser parser = calc_exprParserNew(tokens);

    calc_exprParser_start_return ret = parser->start(parser);

//    pANTLR3_STRING str = ret.tree->toStringTree(ret.tree);
//    pANTLR3_STRING ustr = str->toUTF8(str);

//    const char * xx = (const char*)ustr->chars;

//    printf("tree: %s\n", xx);

//    outTree(0, ret.tree);

    makeCompiler(childAt(ret.tree, 0), result);

    parser->free(parser);
    tokens->free(tokens);
    lex->free(lex);
    input->close(input);
}

}
