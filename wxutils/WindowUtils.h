/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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

#if BOOST_WINDOWS
class AutoScroller {
public:
    explicit AutoScroller(wxWindow * window, wxOrientation orient);
    ~AutoScroller();
private:
    wxWindow * window_;
    wxOrientation orient_;
    SCROLLINFO scrollInfo_;
};
#endif

}
