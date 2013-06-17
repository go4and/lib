#pragma once

#include <boost/optional.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/wstringize.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include <boost/preprocessor/facilities/apply.hpp>

#if !BOOST_WINDOWS
#include <strings.h>
#endif

namespace mstd {

#define MSTD_ENUM_ITEM(s, data, elem) BOOST_PP_CAT(BOOST_PP_APPLY(data), elem),
#define MSTD_ENUM_CASE_NAME(s, data, elem) case BOOST_PP_CAT(BOOST_PP_APPLY(data), elem): return BOOST_PP_STRINGIZE(elem);
#define MSTD_ENUM_CASE_WNAME(s, data, elem) case BOOST_PP_CAT(BOOST_PP_APPLY(data), elem): return BOOST_PP_WSTRINGIZE(elem);
#define MSTD_ENUM_PARSE_ITEM(s, data, elem) if(!BOOST_PP_SEQ_ELEM(1, data)(input, BOOST_PP_SEQ_ELEM(2, data)(elem))) return BOOST_PP_CAT(BOOST_PP_APPLY(BOOST_PP_SEQ_ELEM(0, data)), elem);
#define MSTD_ENUM_PARSE_ITEM_RET(s, data, elem) if(!BOOST_PP_SEQ_ELEM(1, data)(input, BOOST_PP_SEQ_ELEM(2, data)(elem))) { outputValue_ = BOOST_PP_CAT(BOOST_PP_APPLY(BOOST_PP_SEQ_ELEM(0, data)), elem); return true; }
#define MSTD_ENUM_PARSE_LEN_ITEM_RET(s, data, elem) if(len == strlen(BOOST_PP_SEQ_ELEM(2, data)(elem)) && !BOOST_PP_SEQ_ELEM(1, data)(input, BOOST_PP_SEQ_ELEM(2, data)(elem), len)) { outputValue_ = BOOST_PP_CAT(BOOST_PP_APPLY(BOOST_PP_SEQ_ELEM(0, data)), elem); return true; }
#define MSTD_ENUM_TRANSLATOR(enumName, prefix, case, classPrefix) \
    struct BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_APPLY(classPrefix), translate), enumName) { \
        typedef std::BOOST_PP_CAT(BOOST_PP_APPLY(prefix), string) internal_type; \
        typedef enumName external_type; \
        \
        boost::optional<enumName> get_value(const internal_type & v) \
        { \
            return BOOST_PP_CAT(BOOST_PP_CAT(BOOST_PP_APPLY(case), parse), enumName)(v.c_str()); \
        } \
        \
        boost::optional<internal_type> put_value(enumName inp) \
        { \
            return internal_type(BOOST_PP_CAT(BOOST_PP_APPLY(prefix), name)(inp)); \
        } \
    }; \

#if BOOST_WINDOWS
#define MSTD_strcasecmp _stricmp
#define MSTD_wcscasecmp _wcsicmp
#elif defined(ANDROID)
#define MSTD_strcasecmp strcasecmp
#define MSTD_wcscasecmp wcscmp
#else
#define MSTD_strcasecmp strcasecmp
#define MSTD_wcscasecmp wcscasecmp
#endif

#define MSTD_DEFINE_ENUM_IMPL(enumName, prefix, list) \
    enum enumName { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_ITEM, prefix, list) \
    }; \
    \
    inline const char * name(enumName value) { \
        switch(value) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_CASE_NAME, prefix, list); \
        }; \
        return "unknown " BOOST_PP_STRINGIZE(enumName); \
    } \
    \
    inline const wchar_t * wname(enumName value) { \
        switch(value) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_CASE_WNAME, prefix, list); \
        }; \
        return L"unknown " BOOST_PP_WSTRINGIZE(enumName); \
    } \
    \
    inline boost::optional<enumName> BOOST_PP_CAT(parse, enumName)(const char * input) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_PARSE_ITEM, (prefix)(strcmp)(BOOST_PP_STRINGIZE), list); \
        return boost::optional<enumName>(); \
    } \
    \
    inline boost::optional<enumName> BOOST_PP_CAT(parse, enumName)(const wchar_t * input) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_PARSE_ITEM, (prefix)(wcscmp)(BOOST_PP_WSTRINGIZE), list); \
        return boost::optional<enumName>(); \
    } \
    \
    inline boost::optional<enumName> BOOST_PP_CAT(iparse, enumName)(const char * input) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_PARSE_ITEM, (prefix)(MSTD_strcasecmp)(BOOST_PP_STRINGIZE), list); \
        return boost::optional<enumName>(); \
    } \
    \
    inline boost::optional<enumName> BOOST_PP_CAT(iparse, enumName)(const wchar_t * input) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_PARSE_ITEM, (prefix)(MSTD_wcscasecmp)(BOOST_PP_WSTRINGIZE), list); \
        return boost::optional<enumName>(); \
    } \
    \
    inline bool parseEnum(const char * input, enumName & outputValue_) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_PARSE_ITEM_RET, (prefix)(strcmp)(BOOST_PP_STRINGIZE), list); \
        return false; \
    } \
    \
    inline bool parseEnum(const char * input, size_t len, enumName & outputValue_) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_PARSE_LEN_ITEM_RET, (prefix)(strncmp)(BOOST_PP_STRINGIZE), list); \
        return false; \
    } \
    \
    MSTD_ENUM_TRANSLATOR(enumName, BOOST_PP_NIL, BOOST_PP_NIL, BOOST_PP_NIL); \
    MSTD_ENUM_TRANSLATOR(enumName, (w), BOOST_PP_NIL, (w)); \
    MSTD_ENUM_TRANSLATOR(enumName, BOOST_PP_NIL, (i), (i)); \
    MSTD_ENUM_TRANSLATOR(enumName, (w), (i), (iw)); \
    const size_t BOOST_PP_CAT(elementsIn, enumName) = BOOST_PP_SEQ_SIZE(list);
    /**/

#define MSTD_DEFINE_ENUM(enumName, list) MSTD_DEFINE_ENUM_IMPL(enumName, BOOST_PP_NIL, list)
#define MSTD_DEFINE_ENUM_EX(enumName, prefix, list) MSTD_DEFINE_ENUM_IMPL(enumName, (prefix), list)

}
