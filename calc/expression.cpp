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

template<class holded>
class delete_with_me : public mstd::reference_counter<delete_with_me<holded> > {
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
    virtual pre_program * compile(const function_lookup & lookup) const = 0;

    virtual ~pre_compiler()
    {
    }    
};

typedef std::unique_ptr<pre_compiler> pre_compiler_ptr;

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

    pre_program * compile(const function_lookup & lookup) const
    {
        return new Program(lhs->compile(lookup));
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

    pre_program * compile(const function_lookup & lookup) const
    {
        return new Program(lhs->compile(lookup), rhs->compile(lookup));
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
    
    pre_program * compile(const function_lookup & lookup) const
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
    std::vector<pre_compiler_ptr> args;

    pre_program * compile(const function_lookup & lookup) const
    {
        func d = lookup(name, args.size());
        if(d.function.empty())
            throw undefined_function(mstd::utf8(name));
        if(static_cast<size_t>(d.arity) != args.size())
            throw invalid_arity(mstd::utf8(name), d.arity, args.size());
        std::vector<pre_program*> p;
        delete_non_zero dnz(p);
        p.reserve(args.size());
        for(std::vector<pre_compiler_ptr>::const_iterator i = args.begin(), end = args.end(); i != end; ++i)
            p.push_back((*i)->compile(lookup));
        return d.function(p, lookup);
    }
};

void makeInvokation(pANTLR3_BASE_TREE tree, pre_compiler_ptr & out)
{
    InvokationCompiler * result;
    out.reset(result = new InvokationCompiler);
    result->name = getText(tree);
    size_t count = tree->getChildCount(tree);
    result->args.resize(count);
    for(size_t i = 0; i != count; ++i)
        makeCompiler(childAt(tree, i), result->args[i]);
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

    program operator()(const function_lookup & lookup) const
    {
        return wrap_program(impl_->compile(lookup));
    }
private:
    pre_compiler * impl_;
    boost::intrusive_ptr<delete_with_me<pre_compiler> > deleter_;
};

struct parse_context {
    const char * input;
    size_t inputLen;
    boost::function<void()> error;
};

template<class Exception>
class throw_exception {
public:
    throw_exception(const std::wstring & message, const std::wstring & data)
        : message_(message), data_(data)
    {
    }

    void operator()() const
    {
        throw Exception(message_, data_);
    }
private:
    std::wstring message_;
    std::wstring data_;
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

std::wstring make_data(pANTLR3_BASE_RECOGNIZER recognizer, const char * at)
{
    parse_context & context = *static_cast<parse_context*>(recognizer->state->userp);

    if(at < context.input)
        at = context.input;
    else if(at > context.input + context.inputLen)
        at = context.input + context.inputLen;
    size_t prefixLen = std::min<size_t>(at - context.input, 0x20);
    const char * start = eatUtf8Middle(at - prefixLen);
    return mstd::deutf8(start, at - start) + L"<--!!!-->" + mstd::deutf8(at, std::min<size_t>(0x20, context.input + context.inputLen - at));
}

void lexer_recognition_error(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames)
{
    parse_context & context = *static_cast<parse_context*>(recognizer->state->userp);

    pANTLR3_LEXER lexer = static_cast<pANTLR3_LEXER>(recognizer->super);
    pANTLR3_EXCEPTION ex = lexer->rec->state->exception;

    context.error = throw_exception<lexer_exception>(mstd::deutf8(mstd::pointer_cast<const char *>(ex->message)), make_data(recognizer, context.input + ex->charPositionInLine));
}

void display_recognition_error(struct ANTLR3_BASE_RECOGNIZER_struct * recognizer, uint8_t ** tokenNames)
{
    parse_context & context = *static_cast<parse_context*>(recognizer->state->userp);
    if(!context.error.empty())
        return;

    pANTLR3_EXCEPTION ex = recognizer->state->exception;
    pANTLR3_PARSER parser = static_cast<pANTLR3_PARSER>(recognizer->super);
    pANTLR3_INT_STREAM is = parser->tstream->istream;

    /*
        theToken    = (pANTLR3_COMMON_TOKEN)(recognizer->state->exception->token);
        ttext	    = theToken->toString(theToken);

        if  (theToken != NULL)
        {
            if (theToken->type == ANTLR3_TOKEN_EOF)
            {
                ANTLR3_FPRINTF(stderr, ", at <EOF>");
            }
            else
            {
                // Guard against null text in a token
                //
                ANTLR3_FPRINTF(stderr, "\n    near %s\n    ", ttext == NULL ? (pANTLR3_UINT8)"<no text for the token>" : ttext->chars);
            }
        }
        break;
    default:
        ANTLR3_FPRINTF(stderr, "Base recognizer function displayRecognitionError called by unknown parser type - provide override for this function\n");
        return;
        break;
    }
    */
    wchar_t buf[0x40];

    switch (ex->type) {
    case ANTLR3_UNWANTED_TOKEN_EXCEPTION:
        wcscpy(buf, L"Extraneous input");
        break;
    case ANTLR3_MISSING_TOKEN_EXCEPTION:
    case ANTLR3_MISMATCHED_TOKEN_EXCEPTION:
        if(ex->type == ANTLR3_MISSING_TOKEN_EXCEPTION)
            wcscpy(buf, L"Missing ");
        else
            wcscpy(buf, L"Expected ");
        if(tokenNames == NULL)
        {
            wcscat(buf, L"token (");
            mstd::itoa(ex->expecting, buf + wcslen(buf));
            wcscat(buf, L")");
        } else if(ex->expecting == ANTLR3_TOKEN_EOF)
            wcscat(buf, L"<EOF>");
        else {
            const char * token = mstd::pointer_cast<const char*>(tokenNames[ex->expecting]);
            *mstd::deutf8(token, token + strlen(token), buf + wcslen(buf)) = 0;
        }
        break;
    case ANTLR3_RECOGNITION_EXCEPTION:
        wcscpy(buf, L"Recognition error");    
        break;
    case ANTLR3_NO_VIABLE_ALT_EXCEPTION:
        wcscpy(buf, L"Cannot match to any predicted input");
        break;
    case ANTLR3_MISMATCHED_SET_EXCEPTION:
        wcscpy(buf, L"Mismatched set");
        break;
    case ANTLR3_EARLY_EXIT_EXCEPTION:
        wcscpy(buf, L"Missing elements set");
        break;
    default:
        wcscpy(buf, L"Syntax error");
        break;
    }

    context.error = throw_exception<parser_exception>(buf, make_data(recognizer, context.input + is->index(is)));
}

}

void parser::parse(const std::wstring & inp, compiler & result)
{
    char * buffer;
    char * end;
    {
        size_t ilen = inp.length();
        if(!ilen)
            throw empty_input_exception();
        const wchar_t * idata = inp.c_str();
        buffer_.resize(ilen * mstd::max_utf8_length + 1);
        buffer = &buffer_[0];
        end = mstd::utf8(idata, idata + ilen, buffer);
        *end = 0;
    }

    if(input_)
    {
        input_->reuse(input_, pANTLR3_UINT8(buffer), end - buffer, pANTLR3_UINT8("expression"));
        lex_->reset(lex_);
        tokens_->reset(tokens_);
        parser_->reset(parser_);
    } else {
        input_ = antlr3StringStreamNew(pANTLR3_UINT8(buffer), ANTLR3_ENC_UTF8, end - buffer, pANTLR3_UINT8("expression"));
        lex_ = calc_exprLexerNew(input_);
        lex_->pLexer->rec->displayRecognitionError = lexer_recognition_error;
        tokens_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lex_));
        parser_ = calc_exprParserNew(tokens_);
        parser_->pParser->rec->displayRecognitionError = display_recognition_error;
    }
    parse_context context = { buffer, end - buffer };
    lex_->pLexer->rec->state->userp = &context;
    parser_->pParser->rec->state->userp = &context;

    calc_exprParser_start_return ret = parser_->start(parser_);
    if(!context.error.empty())
        context.error();

//    pANTLR3_STRING str = ret.tree->toStringTree(ret.tree);
//    pANTLR3_STRING ustr = str->toUTF8(str);

//    const char * xx = (const char*)ustr->chars;

//    printf("tree: %s\n", xx);

//    outTree(0, ret.tree);

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
