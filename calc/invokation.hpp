#pragma once

#include "fwd.hpp"

namespace calc {

struct invokation_data {
    std::wstring name;
    std::vector<compiler> args;
};

class invokation : public boost::spirit::qi::grammar<std::wstring::const_iterator, invokation_data(), boost::spirit::standard_wide::space_type> {
public:
    invokation(const boost::spirit::qi::rule<std::wstring::const_iterator, compiler(), boost::spirit::standard_wide::space_type> & expr);
    ~invokation();
private:
    class impl;

    boost::spirit::qi::rule<std::wstring::const_iterator, invokation_data(), boost::spirit::standard_wide::space_type> root;
    boost::spirit::qi::rule<std::wstring::const_iterator, std::vector<compiler>(), boost::spirit::standard_wide::space_type> paramList;
    boost::scoped_ptr<impl> impl_;
};

}
