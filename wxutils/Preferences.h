#pragma once

namespace wxutils {

#if BOOST_WINDOWS
mstd::rc_buffer getRegValue(HKEY root, const wchar_t * path, const wchar_t * name);
bool setRegValue(HKEY root, const wchar_t * path, const wchar_t * name, const char * data, size_t size);
#endif

}
