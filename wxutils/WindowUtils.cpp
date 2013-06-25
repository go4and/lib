/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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

void fitSize(wxWindow * rparent, bool width, bool height)
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
            int minX = wxDefaultCoord, minY = wxDefaultCoord, maxX = wxDefaultCoord, maxY = wxDefaultCoord;
            if(width)
            {
                minX = size.x;
                maxX = root->GetMaxWidth();
            }
            if(height)
            {
                minY = size.y;
                maxY = root->GetMaxHeight();
            }
            root->SetSizeHints(minX, minY, maxX, maxY);
            if(!rparent)
                root->SetSize(width ? std::max(size.x, wsize.x) : wsize.x, height ? std::max(size.y, wsize.y) : wsize.y);
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

wxFont defaultGuiFont()
{
    wxFont font;

    HFONT hFont = (HFONT) ::GetStockObject(DEFAULT_GUI_FONT);
    if ( hFont )
    {
        LOGFONT lf;
        if ( ::GetObject(hFont, sizeof(LOGFONT), &lf) != 0 )
        {
            wxNativeFontInfo info;
            info.lf = lf;
            font.Create(info);
        }
    }

    return font;
}

StoredPosition storePosition(wxWindow * window)
{
    WINDOWPLACEMENT wp;
    wp.length = sizeof(wp);
    GetWindowPlacement(window->GetHWND(), &wp);
    StoredPosition result;
    result.maximized = wp.showCmd == SW_MAXIMIZE;
    result.left = wp.rcNormalPosition.left;
    result.top = wp.rcNormalPosition.top;
    result.width = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    result.height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
    if(!window->HasFlag(wxFRAME_TOOL_WINDOW))
    {
        int n = wxDisplay::GetFromWindow(window);
        wxDisplay dpy(n == wxNOT_FOUND ? 0 : n);
        const wxPoint offset = dpy.GetClientArea().GetPosition() - dpy.GetGeometry().GetPosition();
        result.left += offset.x;
        result.top += offset.y;
    }
    return result;
}

void restorePosition(wxWindow * window, const StoredPosition & position)
{
    if(position.left != std::numeric_limits<int>::max())
        window->SetSize(position.left, position.top, position.width, position.height, 0);
    else
        window->SetSize(position.width, position.height);
    if(position.maximized)
    {
        wxTopLevelWindow * tlw = dynamic_cast<wxTopLevelWindow*>(window);
        if(tlw)
            tlw->Maximize();
    }
}
#else

wxFont defaultGuiFont()
{
    return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
}

#endif

}
