#include "pch.h"

namespace wxutils {

void activate(wxTopLevelWindow * window)
{
    [NSApp activateIgnoringOtherApps:YES];
    window->Show(true);
    window->Raise();
    window->SetFocus();
}

wxRect desktopRect()
{
    NSScreen * screen = [NSScreen mainScreen];
    NSRect rect = [screen frame];
    return wxRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

}
