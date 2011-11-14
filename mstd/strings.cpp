#if defined(_MSC_VER)
#pragma warning(disable:4244)
#pragma warning(disable:4996)
#endif

#include <boost/config.hpp>

#if !BOOST_WINDOWS && !__S3E__
#  define MSTD_USE_PBUFFER 1
#else
#  define MSTD_USE_PBUFFER 0
#endif

#if MSTD_USE_PBUFFER
#  include "buffers.hpp"
#else
#  include <vector>
#endif

#if !BOOST_WINDOWS

#  include <boost/algorithm/string.hpp>

#else

#  undef _WIN32_IE
#  define _WIN32_IE _WIN32_IE_IE60SP2
#  include <Windows.h>
#  include <Shlwapi.h>

#endif

#include "pointer_cast.hpp"
#include "utf8.hpp"

#include "strings.hpp"

#if BOOST_WINDOWS
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "user32.lib")
#endif

namespace mstd {

#if !BOOST_WINDOWS

namespace {

class narrow_functor {
public:
    void operator()(const std::ctype<wchar_t> & ct, const wchar_t * begin, const wchar_t * end, char * result)
    {
        ct.narrow(begin, end, '?', result);
    }
};

class widen_functor {
public:
    void operator()(const std::ctype<wchar_t> & ct, const char * begin, const char * end, wchar_t * result)
    {
        ct.widen(begin, end, result);
    }
};

}

template<class OutCh, class InCh, class Func>
std::basic_string<OutCh> process(const std::basic_string<InCh> & value, const std::locale & loc, Func func)
{
    if(!value.empty())
    {
        const std::ctype<wchar_t> & ct = std::use_facet<std::ctype<wchar_t> >(loc);
        std::basic_string<OutCh> result(value.length(), OutCh(0));
        func(ct, value.c_str(), value.c_str() + value.length(), &result[0]);
        return result;
    } else
        return std::basic_string<OutCh>();
}

std::string MSTD_STDCALL narrow(const std::wstring & value)
{
    return process<char>(value, std::locale(), narrow_functor());
}

std::wstring MSTD_STDCALL widen(const std::string & value)
{
    return process<wchar_t>(value, std::locale(), widen_functor());
}

#else

std::string MSTD_STDCALL narrow(const std::wstring & value)
{
    int len = WideCharToMultiByte(CP_ACP, 0, value.c_str(), value.length(), 0, 0, 0, 0);
    if(len > 0)
    {
        std::string res(len, 0);
        WideCharToMultiByte(CP_ACP, 0, value.c_str(), value.length(), &res[0], res.length(), 0, 0);
        return res;
    } else
        return std::string();
}

std::wstring MSTD_STDCALL widen(const std::string & value)
{
    int len = MultiByteToWideChar(CP_ACP, 0, value.c_str(), value.length(), 0, 0);
    if(len > 0)
    {
        std::wstring res(len, 0);
        MultiByteToWideChar(CP_ACP, 0, value.c_str(), value.length(), &res[0], res.length());
        return res;
    } else
        return std::wstring();
}

#endif

namespace {
    const size_t maxStackLen = 0xff;
}

#if MSTD_USE_PBUFFER 
inline char * bufferBegin(const pbuffer & buf)
{
    return buf->ptr();
}
#else
inline char * bufferBegin(std::vector<char> & buf)
{
    return &buf[0];
}
#endif

std::string MSTD_STDCALL utf8(const wchar_t * src, size_t len)
{
    if(len > maxStackLen)
    {
#if MSTD_USE_PBUFFER 
        pbuffer buf = buffers::instance().take(len * max_utf8_length);
#else
        std::vector<char> buf(len * max_utf8_length);
#endif
        char * begin = bufferBegin(buf);
        char * end = mstd::utf8(src, src + len, begin);
        return std::string(begin, end);
    } else if(len) {
        char begin[maxStackLen * max_utf8_length];
        char * end = mstd::utf8(src, src + len, begin);
        return std::string(begin, end);
    } else
        return std::string();
}

std::string MSTD_STDCALL utf8(const wchar_t * src)
{
    return utf8(src, wcslen(src));
}

std::string MSTD_STDCALL utf8(const std::wstring & value)
{
    return utf8(value.c_str(), value.length());
}

inline void utf8_to_lower(std::string & input, size_t len, wchar_t * buffer)
{
    const char * src = input.c_str();
    wchar_t * begin = buffer;
    wchar_t * end = mstd::deutf8(src, src + len, begin);
    to_lower(begin, end - begin);
    input.resize(utf8_length(begin, end));
    char * obegin = &input[0];
    mstd::utf8(begin, end, obegin);
}

void utf8_to_lower(std::string & input)
{
    size_t len = input.length();
    if(len > maxStackLen)
    {
#if MSTD_USE_PBUFFER 
        pbuffer buf = buffers::instance().take(len * sizeof(wchar_t));
#else
        std::vector<char> buf(len * sizeof(wchar_t));
#endif
        utf8_to_lower(input, len, pointer_cast<wchar_t*>(bufferBegin(buf)));
    } else if(len)
    {
        wchar_t buffer[maxStackLen];
        utf8_to_lower(input, len, buffer);
    }
}

inline std::string utf8_to_lower_copy(const std::string & input, size_t len, wchar_t * buffer)
{
    const char * src = input.c_str();
    wchar_t * begin = buffer;
    wchar_t * end = mstd::deutf8(src, src + len, begin);
    to_lower(begin, end - begin);
    std::string out;
    out.resize(utf8_length(begin, end));
    char * obegin = &out[0];
    mstd::utf8(begin, end, obegin);
    return out;
}

std::string utf8_to_lower_copy(const std::string & input)
{
    size_t len = input.length();
    if(len > maxStackLen)
    {
#if MSTD_USE_PBUFFER
        pbuffer buf = buffers::instance().take(len * sizeof(wchar_t));
#else
        std::vector<char> buf(len * sizeof(wchar_t));
#endif
        return utf8_to_lower_copy(input, len, pointer_cast<wchar_t*>(bufferBegin(buf)));
    } else if(len)
    {
        wchar_t buffer[maxStackLen];
        return utf8_to_lower_copy(input, len, buffer);
    } else
        return std::string();
}

class dev_null {
public:
    void operator=(wchar_t) {}
};

class counter : public std::iterator<std::output_iterator_tag, wchar_t> {
public:
    counter() : value_(0) {}

    counter & operator++()
    {
        ++value_;
        return *this;
    }

    size_t value() const
    {
        return value_;
    }

    dev_null operator*()
    {
        return dev_null();
    }
private:
    size_t value_;
};

size_t deutf8_length(const std::string & value)
{
    return deutf8(value.begin(), value.end(), counter()).value();
}

size_t utf8_length(const std::wstring & value)
{
    return mstd::utf8_length(value.begin(), value.end());
}

bool utf8_length_less(const std::string & value, size_t len)
{
    return value.length() < len || deutf8_length(value) < len;
}

std::wstring MSTD_STDCALL deutf8(const char * value)
{
    return mstd::deutf8(value, strlen(value));
}

std::wstring MSTD_STDCALL deutf8(const std::string & value)
{
    return mstd::deutf8(value.c_str(), value.length());
}

std::wstring MSTD_STDCALL deutf8(const char * src, size_t len)
{
    if(len > maxStackLen)
    {
#if MSTD_USE_PBUFFER
        pbuffer buf = buffers::instance().take(len * sizeof(wchar_t));
#else
        std::vector<char> buf(len * sizeof(wchar_t));
#endif
        wchar_t * begin = pointer_cast<wchar_t*>(bufferBegin(buf));
        wchar_t * end = mstd::deutf8(src, src + len, begin);
        return std::wstring(begin, end);
    } else if(len)
    {
        wchar_t begin[maxStackLen];
        wchar_t * end = mstd::deutf8(src, src + len, begin);
        return std::wstring(begin, end);
    } else
        return std::wstring();
}

//////////////////////////////////////////////////////////////////////////
void to_lower(char * str)
{
#if !BOOST_WINDOWS
    boost::to_lower(str);
#else
    CharLowerA(str);
#endif
}

void to_lower(char * str, size_t len)
{
#if !BOOST_WINDOWS
    boost::iterator_range<char*> range(str, str + len);
    boost::to_lower(range);
#else
    CharLowerBuffA(str, len);
#endif
}

void to_lower(wchar_t * str)
{
#if !BOOST_WINDOWS
    boost::to_lower(str);
#else
    CharLowerW(str);
#endif
}

void to_lower(wchar_t * str, size_t len)
{
#if !BOOST_WINDOWS
    boost::iterator_range<wchar_t*> range(str, str + len);
    boost::to_lower(range);
#else
    CharLowerBuffW(str, len);
#endif
}

void to_lower(std::wstring & str)
{
#if !BOOST_WINDOWS
    boost::to_lower(str);
#else
    size_t len = str.length();
    if(len)
        CharLowerBuffW(&str[0], len);
#endif    
}

//////////////////////////////////////////////////////////////////////////
void to_upper(char * str)
{
#if !BOOST_WINDOWS
    boost::to_upper(str);
#else
    CharUpperA(str);
#endif
}

void to_upper(char * str, size_t len)
{
#if !BOOST_WINDOWS
    boost::iterator_range<char*> range(str, str + len);
    boost::to_upper(range);
#else
    CharUpperBuffA(str, len);
#endif
}

void to_upper(wchar_t * str)
{
#if !BOOST_WINDOWS
    boost::to_upper(str);
#else
    CharUpperW(str);
#endif
}

void to_upper(wchar_t * str, size_t len)
{
#if !BOOST_WINDOWS
    boost::iterator_range<wchar_t*> range(str, str + len);
    boost::to_upper(range);
#else
    CharUpperBuffW(str, len);
#endif
}

void to_upper(std::wstring & str)
{
#if !BOOST_WINDOWS
    boost::to_upper(str);
#else
    size_t len = str.length();
    if(len)
        CharUpperBuffW(&str[0], len);
#endif    
}

//////////////////////////////////////////////////////////////////////////
#define TRIM_IMPL(tp, func) \
   tp::iterator i = str.begin(); \
   tp::iterator end = str.end(); \
   while(i != end && ##func(*i)) \
      ++i; \
   if(i != end) \
   { \
      --end; \
      while(##func(*end)) \
         --end; \
      str.erase(++end, str.end()); \
      str.erase(str.begin(), i); \
   } else \
      str.clear();

void trim(std::wstring & str)
{
#if !BOOST_WINDOWS
   boost::trim(str);
#else
   TRIM_IMPL(std::wstring, IsCharSpaceW);
#endif
}

void trim(std::string & str)
{
#if !BOOST_WINDOWS
   boost::trim(str);
#else
   TRIM_IMPL(std::string, IsCharSpaceA);
#endif
}

void trim(const wchar_t *& begin, const wchar_t *& end)
{
#if !BOOST_WINDOWS
    boost::iterator_range<const wchar_t*> r = boost::trim_copy(boost::iterator_range<const wchar_t*>(begin, end));
    begin = r.begin();
    end = r.end();
#else
    const wchar_t * b = begin;
    const wchar_t * e = end;
    while(b != e && IsCharSpaceW(*b))
        ++b;
    if(b != e)
    {
        --e;
        while(IsCharSpaceW(*e))
            --e;
        ++e;
    }
    begin = b;
    end = e;   
#endif    
}

bool iequals(const std::wstring & lhs, const std::wstring & rhs)
{
#if !BOOST_WINDOWS
    return boost::iequals(lhs, rhs);
#else
    const int val = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, lhs.c_str(), lhs.length(), rhs.c_str(), rhs.length()) - 2;
    return !val;
#endif
}

int istrcmp(const std::wstring & lhs, const std::wstring & rhs)
{
#if !BOOST_WINDOWS
    return boost::ilexicographical_compare(lhs, rhs);
#else
    return CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, lhs.c_str(), lhs.length(), rhs.c_str(), rhs.length()) - 2;
#endif
}

}
