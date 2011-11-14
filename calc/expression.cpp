#include "pch.hpp"

#include "fwd.hpp"
#include "environment.hpp"
#include "exception.hpp"
#include "invokation.hpp"

#include "expression.hpp"

namespace ph = boost::phoenix;

namespace calc {

struct invokation_data;

template<typename Iterator, wchar_t quote>
class cstring : public boost::spirit::qi::grammar<Iterator, std::wstring()> {
public:
    cstring()
        : cstring::base_type(root)
    {
        using namespace boost::spirit::labels;
        using boost::spirit::standard_wide::char_;
        root = *(
                  (char_[_val += boost::spirit::_1] - quote - L'\\')
                | (char_(L'\\') >> (
                      char_(quote)[_val += quote]
                    | char_(L'\\')[_val += L'\\']
                  ))
                );
    }
private:
    boost::spirit::qi::rule<Iterator, std::wstring()> root;
};

namespace {

class function_compiler {
public:
    explicit function_compiler(const invokation_data & data)
        : data_(data) {}

    program operator()(const function_lookup & lookup) const
    {
        func d = lookup(data_.name, data_.args.size());
        if(d.function.empty())
            throw undefined_function(mstd::utf8(data_.name));
        if(d.arity != data_.args.size())
            throw invalid_arity(mstd::utf8(data_.name), d.arity, data_.args.size());
        std::vector<program> p;
        p.reserve(data_.args.size());
        for(std::vector<compiler>::const_iterator i = data_.args.begin(), end = data_.args.end(); i != end; ++i)
            p.push_back((*i)(lookup));
        return d.function(p, lookup);
    }
private:
    invokation_data data_;
};

struct make_compiler_t : public mstd::fixed_result_function<compiler> {
    template<class T>
    compiler operator()(const T & t) const
    {
        return ph::lambda[ph::val(t)];
    }
    
    compiler operator()(const invokation_data & data) const
    {
        return function_compiler(data);
    }
};

ph::function<make_compiler_t> make_compiler;

template<class T, class F>
class op {
public:
    explicit op(const program & lhs, const program & rhs, const F & f)
        : lhs_(lhs), rhs_(rhs), f_(f)
    {
    }

    calc::variable operator()(void * ctx, variable * stack) const
    {
        return f_(detail::convert<T>::apply(lhs_(ctx, stack)), detail::convert<T>::apply(rhs_(ctx, stack)));
    }
private:
    program lhs_;
    program rhs_;
    F f_;
};

template<class T, class F>
class op_compiler {
public:
    explicit op_compiler(const compiler & lhs, const compiler & rhs, const F & f)
        : lhs_(lhs), rhs_(rhs), f_(f)
    {
    }

    program operator()(const function_lookup & lookup) const
    {
        return op<T, F>(lhs_(lookup), rhs_(lookup), f_);
    }
private:
    compiler lhs_;
    compiler rhs_;
    F f_;
};

template<class T, class F>
class unary_op {
public:
    explicit unary_op(const program & lhs, const F & f)
        : lhs_(lhs), f_(f)
    {
    }

    calc::variable operator()(void * ctx, variable * stack) const
    {
        const T & t = detail::convert<T>::apply(lhs_(ctx, stack));
        return f_(t);
    }
private:
    program lhs_;
    F f_;
};

template<class T, class F>
class unary_op_compiler {
public:
    explicit unary_op_compiler(const compiler & lhs, const F & f)
        : lhs_(lhs), f_(f)
    {
    }

    program operator()(const function_lookup & lookup) const
    {
        return unary_op<T, F>(lhs_(lookup), f_);
    }
private:
    compiler lhs_;
    F f_;
};

template<class T>
struct make_op_t : public mstd::fixed_result_function<compiler> {
    template<class F>
    compiler operator()(const compiler & lhs, const compiler & rhs, const F & f) const
    {
        return op_compiler<T, F>(lhs, rhs, f);
    }

    template<class F>
    compiler operator()(const compiler & lhs, const F & f) const
    {
        return unary_op_compiler<T, F>(lhs, f);
    }
};

class or_op {
public:
    explicit or_op(const program & lhs, const program & rhs)
        : lhs_(lhs), rhs_(rhs)
    {
    }

    calc::variable operator()(void * ctx, variable * stack) const
    {
        variable v = lhs_(ctx, stack);
        return is_true(v) ? v : rhs_(ctx, stack);
    }
private:
    program lhs_;
    program rhs_;
};

class or_op_compiler {
public:
    explicit or_op_compiler(const compiler & lhs, const compiler & rhs)
        : lhs_(lhs), rhs_(rhs)
    {
    }

    program operator()(const function_lookup & lookup) const
    {
        return or_op(lhs_(lookup), rhs_(lookup));
    }
private:
    compiler lhs_;
    compiler rhs_;
};

struct make_or_op_t : public mstd::fixed_result_function<compiler> {
    compiler operator()(const compiler & lhs, const compiler & rhs) const
    {
        return or_op_compiler(lhs, rhs);
    }
};

class and_op {
public:
    explicit and_op(const program & lhs, const program & rhs)
        : lhs_(lhs), rhs_(rhs)
    {
    }

    calc::variable operator()(void * ctx, variable * stack) const
    {
        variable v = lhs_(ctx, stack);
        return is_true(v) ? rhs_(ctx, stack) : v;
    }
private:
    program lhs_;
    program rhs_;
};

class and_op_compiler {
public:
    explicit and_op_compiler(const compiler & lhs, const compiler & rhs)
        : lhs_(lhs), rhs_(rhs)
    {
    }

    program operator()(const function_lookup & lookup) const
    {
        return and_op(lhs_(lookup), rhs_(lookup));
    }
private:
    compiler lhs_;
    compiler rhs_;
};

struct make_and_op_t : public mstd::fixed_result_function<compiler> {
    compiler operator()(const compiler & lhs, const compiler & rhs) const
    {
        return and_op_compiler(lhs, rhs);
    }
};

ph::function<make_op_t<number> > make_number_op;
ph::function<make_op_t<std::wstring> > make_string_op;
ph::function<make_op_t<variable> > make_var_op;
ph::function<make_or_op_t> make_or_op;
ph::function<make_and_op_t> make_and_op;

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

struct number_policies : public boost::spirit::qi::real_policies<number> {
    static const bool allow_trailing_dot = false;

    template <typename Iterator>
    static bool parse_dot(Iterator & first, Iterator const & last)
    {
        if (first == last || *first != '.')
            return false;
        ++first;
        if(first == last || *first < '0' || *first > '9')
        {
            --first;
            return false;
        }
        return true;
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

}

struct expression_base::impl {
    cstring<std::wstring::const_iterator, L'"'> cstr;
    cstring<std::wstring::const_iterator, L'\''> castr;
    boost::spirit::qi::rule<std::wstring::const_iterator, compiler(), boost::spirit::standard_wide::space_type> expr, 
        oterm, aterm, boterm, bxterm, baterm, eterm, cterm, sterm, bsterm, term, factor;
    invokation invok;

    impl()
        : invok(expr)
    {
        using boost::spirit::labels::_val;
        using boost::spirit::labels::_1;
        using boost::spirit::labels::_2;
        using boost::phoenix::arg_names::arg1;
        using boost::phoenix::arg_names::arg2;
        using boost::spirit::lexeme;
        using boost::spirit::qi::int_parser;
        using boost::spirit::qi::real_parser;
        using boost::spirit::qi::ureal_policies;
        using boost::spirit::qi::uint_parser;
        using boost::phoenix::bind;
        using boost::phoenix::construct;

        expr = oterm[_val = _1] >> *(lexeme[L"or"] >> oterm[_val = make_or_op(_val, _1)]);
        oterm = aterm[_val = _1] >> *(lexeme[L"and"] >> aterm[_val = make_and_op(_val, _1)]);

        aterm = boterm[_val = _1] >> *(L'|' >> boterm[_val = make_number_op(_val, _1, or_<number>())]);
        boterm = bxterm[_val = _1] >> *(L'^' >> bxterm[_val = make_number_op(_val, _1, xor_<number>())]);

        bxterm = baterm[_val = _1] >> *(L'&' >> baterm[_val = make_number_op(_val, _1, and_<number>())]);

        baterm = eterm[_val = _1] >> *(lexeme[L"=="] >> eterm[_val = make_var_op(_val, _1, ph::lambda[arg1 == arg2])]
                                      |lexeme[L"~="] >> eterm[_val = make_var_op(_val, _1, ph::lambda[!(arg1 == arg2)])]);

        eterm = cterm[_val = _1] >> *(lexeme[L">="] >> cterm[_val = make_var_op(_val, _1, ph::lambda[!(arg1 < arg2)])]
                                     |lexeme[L"<="] >> cterm[_val = make_var_op(_val, _1, ph::lambda[!(arg2 < arg1)])]
                                     |L'>' >> cterm[_val = make_var_op(_val, _1, ph::lambda[arg2 < arg1])]
                                     |L'<' >> cterm[_val = make_var_op(_val, _1, ph::lambda[arg1 < arg2])]);

        cterm = sterm[_val = _1] >> *(lexeme[L".."] >> sterm[_val = make_string_op(_val, _1, std::plus<std::wstring>())]);

        sterm = bsterm[_val = _1] >> *((lexeme[L">>"] >> bsterm[_val = make_number_op(_val, _1, shift_right<number>())])
                                      |(lexeme[L"<<"] >> bsterm[_val = make_number_op(_val, _1, shift_left<number>())]));

        bsterm = term[_val = _1] >> *((L'+' >> term[_val = make_number_op(_val, _1, std::plus<number>())])
                                     |(L'-' >> term[_val = make_number_op(_val, _1, std::minus<number>())]));

        term = factor[_val = _1] >> *((L'*' >> factor[_val = make_number_op(_val, _1, std::multiplies<number>())])
                                     |(L'/' >> factor[_val = make_number_op(_val, _1, div())])
                                     |(L'%' >> factor[_val = make_number_op(_val, _1, mod())]));

        factor = (lexeme[L"0x" >> uint_parser<number, 16>()[_val = make_compiler(_1)]])
               | lexeme[int_parser<number>()[_val = make_compiler(_1)]]
               | (lexeme[L"not"] >> factor[_val = make_number_op(_1, std::logical_not<number>())])
               | (lexeme[L'"' >> cstr[_val = make_compiler(_1)] >> '"'])
               | (lexeme[L'\'' >> castr[_val = make_compiler(_1)] >> '\''])
               | L'(' >> expr[_val = _1] >> L')'
               | L'[' >> expr[_val = _1] >> L']'
               | invok[_val = make_compiler(_1)]
               | (L'-' >> expr[_val = make_number_op(_1, std::negate<number>())])
               | (L'+' >> expr[_val = _1])
               | (L'#' >> expr[_val = make_string_op(_1, ph::lambda[ph::size(arg1)])]);

    #if !defined(NDEBUG) && 0
        expr.name("expression.expr");
        oterm.name("expression.oterm");
        aterm.name("expression.aterm");
        cterm.name("expression.cterm");
        sterm.name("expression.sterm");
        term.name("expression.term");
        factor.name("expression.factor");

        debug(expr);
        debug(oterm);
        debug(aterm);
        debug(cterm);
        debug(sterm);
        debug(term);
        debug(factor);
    #endif
    }
};

expression_base::expression_base()
    : impl_(new impl) {}

expression_base::~expression_base()
{
}

const expression_grammar::start_type & expression_base::root() const
{
    return impl_->expr;
}

expression::expression()
    : expression::base_type(root())
{
}

}
