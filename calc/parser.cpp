/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.hpp"

#include "fwd.hpp"
#include "environment.hpp"
#include "error.hpp"

#include "calc_exprLexer.h"
#include "calc_exprParser.h"

#include "parser.hpp"

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

typedef mstd::atomic<size_t> counter;

template<class holded>
class delete_with_me : public mstd::reference_counter<delete_with_me<holded>, mstd::delete_disposer, counter> {
public:
    explicit delete_with_me(holded * h)
        : holded_(h)
    {
    }

    ~delete_with_me()
    {
        delete holded_;
    }
private:
    holded * holded_;
};

class wrap_program {
public:
    explicit wrap_program(pre_program * impl)
        : impl_(impl), deleter_(new delete_with_me<pre_program>(impl))
    {
    }

    variable operator()(void * context, variable * stack) const
    {
        return impl_->run(context, stack);
    }
private:
    pre_program * impl_;
    boost::intrusive_ptr<delete_with_me<pre_program> > deleter_;
};

class pre_compiler : public boost::noncopyable {
public:
    virtual pre_program * compile(const compiler_context & context) const = 0;

    virtual ~pre_compiler()
    {
    }    
};

typedef std::auto_ptr<pre_compiler> pre_compiler_ptr;

void makeCompiler(pANTLR3_BASE_TREE tree, pre_compiler_ptr & out);

template<class Op, class Argument>
struct UnaryProgram : public pre_program {
    Op op;
    pre_program_ptr lhs;
    
    explicit UnaryProgram(pre_program * l)
        : lhs(l)
    {
    }
    
    variable run(void * ctx, variable * stack) const
    {
        return op(detail::convert<Argument>::apply(lhs->run(ctx, stack)));
    }
};

template<class Program>
struct UnaryCompiler : public pre_compiler {
    pre_compiler_ptr lhs;

    pre_program * compile(const compiler_context & context) const
    {
        std::auto_ptr<pre_program> result(lhs->compile(context));
        if(context.err)
            return 0;
        return new Program(result.release());
    }
};

template<class Program>
void makeUnaryCompiler(pANTLR3_BASE_TREE tree, pre_compiler_ptr & out)
{
    BOOST_ASSERT(tree->getChildCount(tree) == 1);
    UnaryCompiler<Program> * result;
    out.reset(result = new UnaryCompiler<Program>());
    makeCompiler(childAt(tree, 0), result->lhs);
}

struct AndProgram : public pre_program {
    pre_program_ptr lhs;
    pre_program_ptr rhs;

    explicit AndProgram(pre_program * l, pre_program * r)
        : lhs(l), rhs(r)
    {
    }

    variable run(void * ctx, variable * stack) const
    {
        variable v = lhs->run(ctx, stack);
        return is_true(v) ? rhs->run(ctx, stack) : v;
    }
};

struct OrProgram : public pre_program {
    pre_program_ptr lhs;
    pre_program_ptr rhs;

    explicit OrProgram(pre_program * l, pre_program * r)
        : lhs(l), rhs(r)
    {
    }

    variable run(void * ctx, variable * stack) const
    {
        variable v = lhs->run(ctx, stack);
        return is_true(v) ? v : rhs->run(ctx, stack);
    }
};

template<class Op, class Argument>
struct BinaryProgram : public pre_program {
    Op op;
    pre_program_ptr lhs;
    pre_program_ptr rhs;
    
    explicit BinaryProgram(pre_program * l, pre_program * r)
        : lhs(l), rhs(r)
    {
    }
    
    variable run(void * ctx, variable * stack) const
    {
        return op(detail::convert<Argument>::apply(lhs->run(ctx, stack)),
                  detail::convert<Argument>::apply(rhs->run(ctx, stack)));
    }
};

template<class Program>
struct BinaryCompiler : public pre_compiler {
    pre_compiler_ptr lhs;
    pre_compiler_ptr rhs;

    pre_program * compile(const compiler_context & context) const
    {
        std::auto_ptr<pre_program> plhs(lhs->compile(context));
        if(context.err)
            return 0;
        std::auto_ptr<pre_program> prhs(rhs->compile(context));
        if(context.err)
            return 0;
        return new Program(plhs.release(), prhs.release());
    }
};

template<class Program>
void makeBinaryCompiler(pANTLR3_BASE_TREE tree, pre_compiler_ptr & out)
{
    BOOST_ASSERT(tree->getChildCount(tree) == 2);
    BinaryCompiler<Program> * result;
    out.reset(result = new BinaryCompiler<Program>());
    makeCompiler(childAt(tree, 0), result->lhs);
    makeCompiler(childAt(tree, 1), result->rhs);
}

template<class Value>
struct ValueProgram : public pre_program {
    Value value;

    explicit ValueProgram(const Value & v)
        : value(v)
    {
    }

    variable run(void * ctx, variable * stack) const
    {
        return value;
    }
};

template<class Value>
struct ValueCompiler : public pre_compiler {
    Value value;

    explicit ValueCompiler(const Value & v)
        : value(v)
    {
    }
    
    pre_program * compile(const compiler_context & context) const
    {
        return new ValueProgram<Value>(value);
    }
};

template<class Value>
void makeValueCompiler(const Value & value, pre_compiler_ptr & out)
{
    out.reset(new ValueCompiler<Value>(value));
}

class delete_non_zero {
public:
    explicit delete_non_zero(std::vector<pre_program*> & p)
        : p_(p)
    {
    }

    ~delete_non_zero()
    {
        for(std::vector<pre_program*>::iterator i = p_.begin(), end = p_.end(); i != end; ++i)
            if(*i)
                delete *i;
    }
private:
    std::vector<pre_program*> & p_;
};

struct InvokationCompiler : public pre_compiler {
    std::wstring name;
    std::vector<pre_compiler*> args;

    pre_program * compile(const compiler_context & context) const
    {
        func d = context.lookup(name, args.size(), true, false);
        if(d.function.empty())
        {
            context.err.init(error_undefined_function).function_name(name).expected_arity(args.size());
            return 0;
        }
        if(static_cast<size_t>(d.arity) != args.size())
        {
            context.err.init(error_invalid_arity).function_name(name).function_arity(d.arity).expected_arity(args.size());
            return 0;
        }
        std::vector<pre_program*> p;
        delete_non_zero dnz(p);
        p.reserve(args.size());
        for(std::vector<pre_compiler*>::const_iterator i = args.begin(), end = args.end(); i != end; ++i)
        {
            p.push_back((*i)->compile(context));
            if(context.err)
                return 0;
        }
        std::auto_ptr<pre_program> result(d.function(p, context));
        if(context.err)
            return 0;
        return result.release();
    }
    
    ~InvokationCompiler()
    {
        for(std::vector<pre_compiler*>::iterator i = args.begin(), end = args.end(); i != end; ++i)
            if(*i)
                delete *i;
    }
};

void makeInvokation(pANTLR3_BASE_TREE tree, pre_compiler_ptr & out)
{
    InvokationCompiler * result;
    out.reset(result = new InvokationCompiler);
    result->name = getText(tree);
    size_t count = tree->getChildCount(tree);
    result->args.reserve(count);
    pre_compiler_ptr temp;
    for(size_t i = 0; i != count; ++i)
    {
        makeCompiler(childAt(tree, i), temp);
        result->args.push_back(temp.release());
    }
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
            ++i;
            auto ch = *i;
            if(ch == 'n')
                ch = '\n';
            else if(ch == 't')
                ch = '\t';
            else if(ch == 'r')
                ch = '\r';
            *w++ = ch;
        } else {
            if(i != w)
                *w = *i;
            ++w;
        }
    }
    result.erase(w, end);
    return result;
}

struct PluginCompiler : public pre_compiler {
    std::wstring value;

    explicit PluginCompiler(const std::wstring & v)
        : value(v)
    {
    }
    
    pre_program * compile(const compiler_context & context) const
    {
        if(!context.plugin.empty())
            return context.plugin(value, context);
        else {
            context.err.init(error_no_plugin_provided);
            return 0;
        }
    }
};

void makePluginCompiler(pANTLR3_UINT8 inp, pre_compiler_ptr & out)
{
    const char * temp = mstd::pointer_cast<const char*>(inp);
    out.reset(new PluginCompiler(mstd::deutf8(temp)));
}

void makeCompiler(pANTLR3_BASE_TREE tree, pre_compiler_ptr & out)
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
        makeUnaryCompiler<UnaryProgram<size_, std::wstring> >(tree, out);
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
            makeUnaryCompiler<UnaryProgram<std::negate<number>, number> >(tree, out);
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
        makeUnaryCompiler<UnaryProgram<std::logical_not<number>, number> >(tree, out);
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
    case T_CB_EXPR:
        makePluginCompiler(tree->getText(tree)->chars, out);
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

class wrap_compiler {
public:
    explicit wrap_compiler(pre_compiler_ptr & impl)
        : impl_(impl.release()), deleter_(new delete_with_me<pre_compiler>(impl_))
    {
    }

    program operator()(const compiler_context & context) const
    {
        std::auto_ptr<pre_program> p(impl_->compile(context));
        if(context.err)
            return program();
        return wrap_program(p.release());
    }
private:
    pre_compiler * impl_;
    boost::intrusive_ptr<delete_with_me<pre_compiler> > deleter_;
};

struct parse_context {
    const char * input;
    size_t inputLen;
    error * err;
};

const char * eatUtf8Middle(const char * inp)
{
    for(;; ++inp)
    {
        unsigned char c = *inp;
        if((c & 0xc0) != 0x80)
            return inp;
    }
}

std::string make_data(pANTLR3_BASE_RECOGNIZER recognizer, const char * at)
{
    parse_context & context = *static_cast<parse_context*>(recognizer->state->userp);

    if(at < context.input)
        at = context.input;
    else if(at > context.input + context.inputLen)
        at = context.input + context.inputLen;
    size_t prefixLen = std::min<size_t>(at - context.input, 0x20);
    const char * start = eatUtf8Middle(at - prefixLen);
    return std::string(start, at - start) + "<--!!!-->" + std::string(at, std::min<size_t>(0x20, context.input + context.inputLen - at));
}

void lexer_recognition_error(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames)
{
    parse_context & context = *static_cast<parse_context*>(recognizer->state->userp);

    pANTLR3_LEXER lexer = static_cast<pANTLR3_LEXER>(recognizer->super);
    pANTLR3_EXCEPTION ex = lexer->rec->state->exception;

    context.err->init(error_lexer).location(make_data(recognizer, context.input + ex->charPositionInLine)).message(mstd::pointer_cast<const char *>(ex->message));
}

void display_recognition_error(struct ANTLR3_BASE_RECOGNIZER_struct * recognizer, uint8_t ** tokenNames)
{
    parse_context & context = *static_cast<parse_context*>(recognizer->state->userp);
    if(*context.err)
        return;

    pANTLR3_EXCEPTION ex = recognizer->state->exception;

    switch (ex->type) {
    case ANTLR3_UNWANTED_TOKEN_EXCEPTION:
        context.err->init(error_extraneous_input);
        break;
    case ANTLR3_MISSING_TOKEN_EXCEPTION:
    case ANTLR3_MISMATCHED_TOKEN_EXCEPTION:
        context.err->init(ex->type == ANTLR3_MISSING_TOKEN_EXCEPTION ? error_missing_token : error_mismatched_token);
        if(tokenNames == NULL)
            context.err->token_id(ex->expecting);
        else if(ex->expecting == ANTLR3_TOKEN_EOF)
            context.err->token_eof(true);
        else
            context.err->token_value(mstd::pointer_cast<const char*>(tokenNames[ex->expecting]));
        break;
    case ANTLR3_RECOGNITION_EXCEPTION:
        context.err->init(error_recognition);
        break;
    case ANTLR3_NO_VIABLE_ALT_EXCEPTION:
        context.err->init(error_no_viable_alt);
        break;
    case ANTLR3_MISMATCHED_SET_EXCEPTION:
        context.err->init(error_mismatched_set);
        break;
    case ANTLR3_EARLY_EXIT_EXCEPTION:
        context.err->init(error_early_exit);
        break;
    default:
        context.err->init(error_syntax);
        break;
    }
}

}

void parser::parse(const std::string & inp, compiler & result, error & err)
{
    size_t ilen = inp.length();
    if(!ilen)
    {
        err.init(error_empty_input);
        return;
    }
    buffer_.resize(ilen + 1);
    memcpy(&buffer_[0], inp.c_str(), ilen + 1);
    do_parse(ilen, result, err);
}

void parser::parse(const std::wstring & inp, compiler & result, error & err)
{
    char * buffer;
    char * end;
    {
        size_t ilen = inp.length();
        if(!ilen)
        {
            err.init(error_empty_input);
            return;
        }
        const wchar_t * idata = inp.c_str();
        buffer_.resize(ilen * mstd::max_utf8_length + 1);
        buffer = &buffer_[0];
        end = mstd::utf8(idata, idata + ilen, buffer);
        *end = 0;
    }
    do_parse(end - buffer, result, err);
}

void parser::do_parse(size_t len, compiler & result, error & err)
{  
    char * buffer = &buffer_[0];
    if(input_)
    {
        input_->reuse(input_, pANTLR3_UINT8(buffer), len, pANTLR3_UINT8("expression"));
        lex_->reset(lex_);
        tokens_->reset(tokens_);
        parser_->reset(parser_);
    } else {
        input_ = antlr3StringStreamNew(pANTLR3_UINT8(buffer), ANTLR3_ENC_UTF8, len, pANTLR3_UINT8("expression"));
        lex_ = calc_exprLexerNew(input_);
        lex_->pLexer->rec->displayRecognitionError = lexer_recognition_error;
        tokens_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lex_));
        parser_ = calc_exprParserNew(tokens_);
        parser_->pParser->rec->displayRecognitionError = display_recognition_error;
    }
    parse_context context = { buffer, len, &err };
    lex_->pLexer->rec->state->userp = &context;
    parser_->pParser->rec->state->userp = &context;

    calc_exprParser_start_return ret = parser_->start(parser_);
    if(err)
        return;

    pre_compiler_ptr preResult;
    makeCompiler(childAt(ret.tree, 0), preResult);
    result = wrap_compiler(preResult);
}

parser::~parser()
{
    if(input_)
    {
        parser_->free(parser_);
        tokens_->free(tokens_);
        lex_->free(lex_);
        input_->close(input_);
    }
}

}
