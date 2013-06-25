/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#define MSTD_DEFINE_ENUM(enumName, list) MSTD_DEFINE_ENUM_IMPL(enumName, BOOST_PP_NIL, list)
#define MSTD_DEFINE_ENUM_EX(enumName, prefix, list) MSTD_DEFINE_ENUM_IMPL(enumName, (prefix), list)

}
