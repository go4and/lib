#include "pch.h"

#include "ScopedDialog.h"

namespace wxutils {

ScopedDialogBase::ScopedDialogBase()
    : window_(0) {}

ScopedDialogBase::~ScopedDialogBase()
{
    reset(0);
}

ScopedDialogBase::operator safe_bool() const
{
    return window_ ? &Dummy::f : 0;
}

wxWindow * ScopedDialogBase::window() const
{
    return window_;
}

void ScopedDialogBase::reset(wxWindow * window)
{
    if(window_)
        window_->Close(true);
    window_ = window;
    if(window_)
    {
        window_->Connect(wxEVT_DESTROY, wxWindowDestroyEventHandler(ScopedDialogBase::OnDestroy), 0, this);
        window_->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ScopedDialogBase::OnClose), 0, this);
    }
}

void ScopedDialogBase::forget()
{
    window_ = 0;
}

void ScopedDialogBase::OnDestroy(wxWindowDestroyEvent & event)
{
    forget();
    event.Skip();
}

void ScopedDialogBase::OnClose(wxCloseEvent & event)
{
    if(window_)
        window_->Destroy();
    event.Skip();
}

}
