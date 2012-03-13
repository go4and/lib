#pragma once

#include "Config.h"

class wxTopLevelWindow;
class wxWindow;

namespace wxutils {

WXUTILS_DECL void activate(wxTopLevelWindow * window);
WXUTILS_DECL void fitWidth(wxWindow * window);
WXUTILS_DECL wxTopLevelWindow * root(wxWindow * window);
WXUTILS_DECL wxRect desktopRect();

}
