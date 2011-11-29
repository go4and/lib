#pragma once

#include <boost/optional.hpp>

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/wstringize.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

namespace mstd {

#define MSTD_ENUM_ITEM(s, data, elem) BOOST_PP_CAT(data, elem),
#define MSTD_ENUM_CASE_NAME(s, data, elem) case BOOST_PP_CAT(data, elem): return BOOST_PP_STRINGIZE(elem);
#define MSTD_ENUM_CASE_WNAME(s, data, elem) case BOOST_PP_CAT(data, elem): return BOOST_PP_WSTRINGIZE(elem);
#define MSTD_ENUM_PARSE_ITEM(s, data, elem) if(!strcmp(input, BOOST_PP_STRINGIZE(elem))) return BOOST_PP_CAT(data, elem);
#define MSTD_ENUM_PARSE_WITEM(s, data, elem) if(!wcscmp(input, BOOST_PP_WSTRINGIZE(elem))) return BOOST_PP_CAT(data, elem);
#define MSTD_ENUM_TRANSLATOR(enumName, prefix) \
    struct BOOST_PP_CAT(BOOST_PP_CAT(prefix, translate), enumName) { \
        typedef std::BOOST_PP_CAT(prefix, string) internal_type; \
        typedef enumName external_type; \
        \
        boost::optional<enumName> get_value(const internal_type & v) \
        { \
            return BOOST_PP_CAT(parse, enumName)(v.c_str()); \
        } \
        \
        boost::optional<internal_type> put_value(enumName inp) \
        { \
            return internal_type(BOOST_PP_CAT(prefix, name)(inp)); \
        } \
    }; \

#define MSTD_DEFINE_ENUM_EX(enumName, prefix, list) \
    enum enumName { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_ITEM, prefix, list) \
    }; \
    \
    inline const char * name(enumName value) { \
        switch(value) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_CASE_NAME, prefix, list); \
        }; \
        return "unknown "BOOST_PP_STRINGIZE(enumName); \
    } \
    \
    inline const wchar_t * wname(enumName value) { \
        switch(value) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_CASE_WNAME, prefix, list); \
        }; \
        return L"unknown "BOOST_PP_WSTRINGIZE(enumName); \
    } \
    \
    inline boost::optional<enumName> BOOST_PP_CAT(parse, enumName)(const char * input) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_PARSE_ITEM, prefix, list); \
        return boost::optional<enumName>(); \
    } \
    \
    inline boost::optional<enumName> BOOST_PP_CAT(parse, enumName)(const wchar_t * input) { \
        BOOST_PP_SEQ_FOR_EACH(MSTD_ENUM_PARSE_WITEM, prefix, list); \
        return boost::optional<enumName>(); \
    } \
    \
    MSTD_ENUM_TRANSLATOR(enumName, ); \
    MSTD_ENUM_TRANSLATOR(enumName, w); \
    const size_t BOOST_PP_CAT(elementsIn, enumName) = BOOST_PP_SEQ_SIZE(list);
    /**/

#define MSTD_DEFINE_ENUM(enumName, list) MSTD_DEFINE_ENUM_EX(enumName, , list)

}
