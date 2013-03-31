#pragma once

#include <string>

#include "config.hpp"

namespace mstd {

namespace detail {

template<class T> struct switch_char;

template<> struct switch_char<char> { typedef wchar_t type; };
template<> struct switch_char<wchar_t> { typedef char type; };

}

template<class Ch>
std::basic_string<typename detail::switch_char<Ch>::type> convert(const std::basic_string<Ch> & src)
{
    return std::basic_string<typename detail::switch_char<Ch>::type>(src.begin(), src.end());
}

MSTD_DECL std::string MSTD_STDCALL narrow(const std::wstring & value);
MSTD_DECL std::wstring MSTD_STDCALL widen(const std::string & value);

MSTD_DECL std::string MSTD_STDCALL utf8(const std::wstring & value);
MSTD_DECL std::string MSTD_STDCALL utf8(const wchar_t * src);
MSTD_DECL std::string MSTD_STDCALL utf8(const wchar_t * src, size_t len);

MSTD_DECL size_t deutf8_length(const std::string & value);
MSTD_DECL size_t utf8_length(const std::wstring & value);
MSTD_DECL bool utf8_length_less(const std::string & value, size_t len);

MSTD_DECL std::wstring MSTD_STDCALL deutf8(const std::string & value);
MSTD_DECL std::wstring MSTD_STDCALL deutf8(const char * value);
MSTD_DECL std::wstring MSTD_STDCALL deutf8(const char * value, size_t len);

MSTD_DECL std::string utf8_to_lower_copy(const std::string & input);
MSTD_DECL void utf8_to_lower(std::string & str);
MSTD_DECL bool utf8_iequals(const std::string & lhs, const std::string & rhs);

struct deutf8_functor {
    std::wstring operator()(const std::string & input) const
    {
        return deutf8(input);
    }
};

struct utf8_functor {
    std::string operator()(const std::wstring & input) const
    {
        return utf8(input);
    }
};

inline deutf8_functor deutf8()
{
    return deutf8_functor();
}

inline utf8_functor utf8()
{
    return utf8_functor();
}

void to_lower(std::wstring & inp);
void to_lower(wchar_t * inp);
void to_lower(wchar_t * inp, size_t len);

template<class Str>
Str to_lower_copy(Str inp)
{
    to_lower(inp);
    return inp;
}

void to_upper(std::string & inp);
void to_upper(char * inp);
void to_upper(char * inp, size_t len);
void to_upper(std::wstring & inp);
void to_upper(wchar_t * inp);
void to_upper(wchar_t * inp, size_t len);

template<class Str>
Str to_upper_copy(Str inp)
{
    to_upper(inp);
    return inp;
}

void trim(std::string & inp);
void trim(const char *& begin, const char *& end);
void trim(std::wstring & inp);
void trim(std::wstring & inp);
void trim(const wchar_t *& begin, const wchar_t *& end);

template<class Ch>
void trim(std::pair<const Ch*, const Ch*> & p)
{
    trim(p.first, p.second);
}

template<class Str>
Str trim_copy(Str inp)
{
    trim(inp);
    return inp;
}

bool iequals(const std::wstring & lhs, const std::wstring & rhs);
bool iequals(const std::string & lhs, const std::string & rhs);

int istrcmp(const std::wstring & lhs, const std::wstring & rhs);
int istrcmp(const std::string & lhs, const std::string & rhs);

template<class Output, class It, class Value>
void split_args_impl(Output & output, It input, It end, const Value &)
{
    while(input != end && *input == ' ')
        ++input;
    if(input == end)
        return;
    It begin = input;
    It out = begin;
    while(input != end)
    {
        Value ch = *input++;
        if(ch == '\\')
        {
            if(input == end)
                break;
            *out++ = *input;
            ++input;
        } else if(ch == '"' || ch == '\'')
        {
            while(input != end)
            {
                Value cc = *input;
                ++input;
                if(cc == '\\')
                {
                    if(input == end)
                        break;
                    *out++ = *input;
                    ++input;
                } else if(cc == ch)
                    break;
                else
                    *out++ = cc;
            }
        } else if(ch == ' ')
        {
            output.push_back(typename Output::value_type(begin, out));
            out = begin;
            while(input != end && *input == ' ')
                ++input;
            if(input == end)
                return;
        } else
            *out++ = ch;
    }
    output.push_back(typename Output::value_type(begin, out));
}

template<class Output, class It>
void split_args(Output & output, It input, It end)
{
    if(input != end)
        split_args_impl(output, input, end, *input);
}

}
