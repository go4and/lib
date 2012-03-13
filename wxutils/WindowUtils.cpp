#include "pch.h"

#include "WindowUtils.h"

namespace wxutils {

#if BOOST_WINDOWS
void activate(wxTopLevelWindow * window)
{
    window->Show(true);
    window->Raise();
    HWND hwnd = static_cast<HWND>(window->GetHWND());
    SetWindowPos(hwnd, HWND_TOP, -1, -1, -1, -1, SWP_NOSIZE | SWP_NOMOVE);

    HWND hwndActive = GetForegroundWindow();
    DWORD dwActiveThreadID = GetWindowThreadProcessId(hwndActive, NULL);
    DWORD dwOurThreadID = GetCurrentThreadId();
    if(dwOurThreadID != dwActiveThreadID)
        AttachThreadInput(dwOurThreadID, dwActiveThreadID, true);
    SetForegroundWindow(hwnd);
}
#endif

void fitWidth(wxWindow * rparent)
{
    wxWindow * root = 0;

    while(rparent != 0)
    {
        root = rparent;
        rparent = root->GetParent();
        root->Layout();
        if(root->GetSizer())
        {
            const wxSize size = root->GetSizer()->ComputeFittingWindowSize(root);
            const wxSize wsize = root->GetSize();
            root->SetSizeHints(size.x, wxDefaultCoord, root->GetMaxWidth(), wxDefaultCoord);
            if(!rparent)
                root->SetSize(std::max(size.x, wsize.x), wsize.y);
        }
        root->Refresh(true);
    }
}


wxTopLevelWindow * root(wxWindow * window)
{
    while(window && !window->IsTopLevel())
        window = window->GetParent();
    return boost::polymorphic_downcast<wxTopLevelWindow*>(window);
}

#if BOOST_WINDOWS
wxRect desktopRect()
{
    return wxRect(GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
                  GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));
}
#endif

}
