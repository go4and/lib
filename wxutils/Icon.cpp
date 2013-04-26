#include "pch.h"

#include "Instance.h"
#include "Icon.h"

namespace wxutils {

#if BOOST_WINDOWS
wxIcon makeIcon(unsigned int resourceId, int sx, int sy)
{
    if(sy == -1)
        sy = sx;
    HICON hIcon = static_cast<HICON>(::LoadImage(instance(), MAKEINTRESOURCE(resourceId), IMAGE_ICON, sx, sy, 0));
    wxIcon icon;
    icon.SetHICON(hIcon);
    BOOST_ASSERT(icon.IsOk());
    icon.SetSize(wxSize(sx, sy));
    
    return icon;
}

WXUTILS_DECL void addIcon(wxIconBundle & icons, unsigned int resourceId, int sx, int sy)
{
    icons.AddIcon(makeIcon(resourceId, sx, sy));
}

#endif

}
