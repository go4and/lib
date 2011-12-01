#include "pch.hpp"

#include "fwd.hpp"

#include "utils.hpp"

namespace calc {

bool is_false(const variable & any)
{
    return is<number>(any, 0);
}

struct ToStringVisitor : public boost::static_visitor<std::wstring> {
public:
    std::wstring operator()(calc::number i) const
    {
        wchar_t buf[0x20];
        return mstd::itoa(i, buf);
    }
    
    const std::wstring & operator()(const std::wstring & s) const
    {
        return s;
    }
};

std::wstring to_string(const variable & var)
{
    return boost::apply_visitor(ToStringVisitor(), var);
}

struct ToNumberVisitor : public boost::static_visitor<calc::number> {
public:
    calc::number operator()(calc::number i) const
    {
        return i;
    }
    
    calc::number operator()(const std::wstring & s) const
    {
        try {
            return mstd::str2int10_checked<calc::number>(s);
        } catch(mstd::bad_str2int_cast&) {
            return 0;
        }
    }
};

calc::number to_number(const variable & var)
{
    return boost::apply_visitor(ToNumberVisitor(), var);
}

const number * as_number(const variable & var)
{
    return boost::get<number>(&var);
}

const std::wstring * as_string(const variable & var)
{
    return boost::get<std::wstring>(&var);
}

}
