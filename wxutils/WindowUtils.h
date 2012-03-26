#pragma once

#include "Config.h"

class wxTopLevelWindow;
class wxWindow;

namespace wxutils {

WXUTILS_DECL void activate(wxTopLevelWindow * window);

WXUTILS_DECL void fitSize(wxWindow * window, bool width, bool height);
inline void fitWidth(wxWindow * window) { fitSize(window, true, false); }
inline void fitHeight(wxWindow * window) { fitSize(window, false, true); }

WXUTILS_DECL wxTopLevelWindow * root(wxWindow * window);
WXUTILS_DECL wxRect desktopRect();

}
