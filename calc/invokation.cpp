#include "pch.hpp"

#include "identifier.hpp"

#include "invokation.hpp"

namespace ph = boost::phoenix;

BOOST_FUSION_ADAPT_STRUCT(
    calc::invokation_data,
    (std::wstring, name)
    (std::vector<calc::compiler>, args)
)

namespace calc {

class invokation::impl {
public:
    identifier<std::wstring::const_iterator> iden;
};

invokation::invokation(const boost::spirit::qi::rule<std::wstring::const_iterator, compiler(), boost::spirit::standard_wide::space_type> & expr)
    : invokation::base_type(root)
{
    using boost::spirit::labels::_val;
    using boost::spirit::labels::_1;
    using boost::spirit::labels::_2;
    using boost::phoenix::arg_names::arg1;
    using boost::phoenix::arg_names::arg2;
    using boost::spirit::lexeme;
    using boost::phoenix::at_c;

    impl_.reset(new impl);

    root = lexeme[impl_->iden][at_c<0>(_val) = _1] >>
                 -(   ('[' >> paramList[at_c<1>(_val) = _1] >> ']')
                    | ('(' >> paramList[at_c<1>(_val) = _1] >> ')'));

    paramList = expr[ph::push_back(_val, _1)] >> *(',' >> expr[ph::push_back(_val, _1)]);

    root.name("invokation.root");
    paramList.name("invokation.paramList");
}

invokation::~invokation()
{
}

}
