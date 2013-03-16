#include "src/pch.hpp"

#include "parsers.hpp"

namespace mptree {

bool parse_value(std::wstring & out, const char * value)
{
    out = mstd::deutf8(value);
    return true;
}

bool parse_value(bool & out, const char * value)
{
    if(!strcmp(value, "1") || !strcmp(value, "true"))
        out = true;
    else if(!strcmp(value, "0") || !strcmp(value, "false"))
        out = false;
    else
        return false;
    return true;
}

bool parse_value(int & out, const char * value)
{
    try {
        out = mstd::str2int10_checked<int>(value);
        return true;
    } catch(mstd::bad_str2int_cast &) {
        return false;
    }
}

bool parse_value(double & out, const char * val)
{
    char * endptr = 0;
    double value = strtod(val, &endptr);
    if(*endptr)
        return false;
    out = value;
    return true;
}

}
