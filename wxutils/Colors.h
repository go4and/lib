/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#if !defined(BUILDING_WXUTILS)
#include <boost/unordered_map.hpp>

#include <mstd/singleton.hpp>

#include <wx/colour.h>
#endif

#include "Config.h"

namespace wxutils {

wxColor string2color(const std::string & value);
wxColor string2color(const std::wstring & value);
std::wstring color2string(const wxColor & value);

}
