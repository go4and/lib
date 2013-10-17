/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#if !defined(_STLP_NO_IOSTREAMS)

#if defined(_MSC_VER)
#pragma warning(disable: 4100)
#pragma warning(disable: 4127)
#pragma warning(disable: 4512)
#endif

#if !__APPLE__
#include <boost/spirit/home/qi.hpp>
#else
#include <string>
#endif

#include "email.hpp"

namespace mstd {

#if __APPLE__
bool verifyEmail(const std::string & input)
{
    return true;
}

#else
    
template<class It>
class email_grammar : public boost::spirit::qi::grammar<It> {
public:
    email_grammar()
        : email_grammar::base_type(e_mailbox)
    {
        using namespace boost::spirit;
        using namespace boost::spirit::qi;
#if BOOST_VERSION < 104100
        using namespace boost::spirit::ascii;
#endif

        e_mailbox = lexeme[e_local_part >> e_at_domain];
        e_at_domain = '@' >>  e_domain;
        e_domain = e_name >> *('.' >> e_name);
        e_local_part = alnum >> -char_('.') >> -(+e_char >> *('.' >> +e_char));
        e_name = alpha >> +(*char_('-') >> alnum);
        e_char = char_('\041', '\177') - e_special;
        e_special = char_('<') | '>' | '(' | ')' | '[' | ']' | '\\' | '.' | ',' | ';' | ':' | '@' | '\"' | cntrl;
    }
private:
    typename email_grammar::start_type
        e_mailbox, e_at_domain, e_domain, e_local_part,
        e_name, e_char, e_special, e_name_middle;
};

template<class It>
bool verifyEmail(It begin, It end)
{
    mstd::email_grammar<It> grammar;
    return (end - begin < 64) && parse(begin, end, grammar >> boost::spirit::eoi);
}

bool verifyEmail(const std::string & input)
{
    return verifyEmail(input.begin(), input.end());
}

#endif
    
}

#endif
