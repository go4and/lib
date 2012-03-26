#include "pch.h"

namespace wxutils {

void activate(wxTopLevelWindow * window)
{
    [NSApp activateIgnoringOtherApps:YES];
    window->Show(true);
    window->Raise();
    window->SetFocus();
}

}
