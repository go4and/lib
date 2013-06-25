/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#if !defined(BUILDING_WXUTILS)
#include <string>
#endif

#include "Config.h"

class wxGridBagSizer;
class wxWindow;

namespace wxutils {

WXUTILS_DECL void addSeparator(wxGridBagSizer * psizer, int y, wxWindow * parent, const std::wstring & title);

}
