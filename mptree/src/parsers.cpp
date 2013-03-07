#include "pch.hpp"

#include "parsers.hpp"

namespace mptree {

bool parse_value(std::wstring & out, const char * value, size_t len)
{
    out = mstd::deutf8(value, len);
    return true;
}

bool parse_value(bool & out, const char * value, size_t len)
{
    if(len == 1)
    {
        if(*value == '1')
            out = true;
        else if(*value == '0')
            out = false;
        else
            return false;
        return true;
    } else if(len == 4 && !strncmp(value, "true", len))
        out = true;
    else if(len == 5 && !strncmp(value, "false", len))
        out = false;
    else
        return false;
    return true;
}

bool parse_value(int & out, const char * value, size_t len)
{
    try {
        out = mstd::str2int10_checked<int>(value, len);
        return true;
    } catch(mstd::bad_str2int_cast & exc) {
        return false;
    }
}

bool parse_value(double & out, const char * val, size_t len)
{
    char buffer[0x40];
    if(len >= sizeof(buffer))
        return false;
    memcpy(buffer, val, len);
    buffer[len] = 0;
    char * endptr = 0;
    double value = strtod(buffer, &endptr);
    if(*endptr)
        return false;
    out = value;
    return true;
}

}
