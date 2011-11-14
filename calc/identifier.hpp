#pragma once

#if !defined(BUILDING_CALC)
#include <boost/spirit/home/qi/nonterminal/grammar.hpp>
#include <boost/spirit/home/qi/operator.hpp>

#include <boost/spirit/home/phoenix/operator/arithmetic.hpp>
#endif

namespace calc {

template <typename Iterator>
class identifier : public boost::spirit::qi::grammar<Iterator, std::wstring()> {
public:
    identifier()
        : identifier::base_type(root)
    {
        using boost::spirit::standard_wide::alpha;
        using boost::spirit::standard_wide::alnum;
        using boost::spirit::standard_wide::char_;

        root = alpha[boost::spirit::labels::_val += boost::spirit::labels::_1] >> 
               *(alnum[boost::spirit::labels::_val += boost::spirit::labels::_1] | char_('_')[boost::spirit::labels::_val += L'_']);

        root.name("identifier.root");
    }
private:
    boost::spirit::qi::rule<Iterator, std::wstring()> root;
};

}
