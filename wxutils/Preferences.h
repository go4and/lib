/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace wxutils {

#if BOOST_WINDOWS
mstd::rc_buffer getRegValue(HKEY root, const wchar_t * path, const wchar_t * name);
bool setRegValue(HKEY root, const wchar_t * path, const wchar_t * name, const char * data, size_t size);
#endif

}
