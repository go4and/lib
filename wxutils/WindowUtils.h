#pragma once

#include "Config.h"

class wxTopLevelWindow;
class wxWindow;

namespace wxutils {

WXUTILS_DECL void activate(wxTopLevelWindow * window);

WXUTILS_DECL void fitSize(wxWindow * window, bool width = true, bool height = true);
inline void fitWidth(wxWindow * window) { fitSize(window, true, false); }
inline void fitHeight(wxWindow * window) { fitSize(window, false, true); }

WXUTILS_DECL wxTopLevelWindow * root(wxWindow * window);
WXUTILS_DECL wxRect desktopRect();

WXUTILS_DECL wxFont defaultGuiFont();

struct StoredPosition {
    bool maximized;
    int left;
    int top;
    int width;
    int height;
};

StoredPosition storePosition(wxWindow * window);
void restorePosition(wxWindow * window, const StoredPosition & position);

}
