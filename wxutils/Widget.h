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

class wxWindow;
class wxSizer;
class wxGridBagSizer;
class wxSizerItem;
class wxGBSizerItem;

namespace wxutils {

class WXUTILS_DECL Widget {
private:
    struct Dummy { void f() {} };
public:
    typedef void (Dummy::*safe_bool)();

    Widget();
    Widget(wxWindow * window);
    Widget(wxSizer * sizer);
    
    wxSizerItem * addTo(wxSizer * sizer);
    wxGBSizerItem * addTo(wxGridBagSizer * sizer);
    void destroy(wxSizer * sizer);

    operator safe_bool() const;
private:
    int which_;
    void * pointer_;
    
    class Visitor;
    friend class Visitor;
};

}
