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
#include <boost/cast.hpp>
#include <boost/noncopyable.hpp>

#include <wx/event.h>
#endif

#include "Config.h"

class wxWindow;

namespace wxutils {

class WXUTILS_DECL ScopedDialogBase : public wxEvtHandler, private boost::noncopyable {
private:
    struct Dummy { void f() {} };
public:
    typedef void (Dummy::*safe_bool)();
    
    ScopedDialogBase();
    ~ScopedDialogBase();

    operator safe_bool() const;
protected:
    wxWindow * window() const;
    void reset(wxWindow * window);
private:
    void forget();
    void OnDestroy(wxWindowDestroyEvent & event);
    void OnClose(wxCloseEvent & event);

    wxWindow * window_;
};

template<class T>
class ScopedDialog : public ScopedDialogBase {
public:
    ScopedDialog(T * t = 0)
    {
        reset(t);
    }

    T * operator->() const
    {
        return boost::polymorphic_downcast<T*>(window());
    }
    
    void reset(T * t = 0)
    {
        ScopedDialogBase::reset(t);
    }

    T * get() const
    {
        return boost::polymorphic_downcast<T*>(window());
    }
};

}
