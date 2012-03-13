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
