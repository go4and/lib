#pragma once

#if !defined(BUILDING_CALC)
#include <boost/scoped_ptr.hpp>

#include <boost/spirit/home/support.hpp>

#include <boost/spirit/home/qi/nonterminal/grammar.hpp>
#include <boost/spirit/home/qi/nonterminal/rule.hpp>
#endif

#include "fwd.hpp"

namespace calc {

typedef boost::spirit::qi::grammar<std::wstring::const_iterator, compiler(), boost::spirit::standard_wide::space_type> expression_grammar;

class expression_base {
protected:
    expression_base();
    ~expression_base();
    
    const expression_grammar::start_type & root() const;
private:
    struct impl;
    boost::scoped_ptr<impl> impl_;
};

class expression : private expression_base, public expression_grammar {
public:
    expression();
};

}
