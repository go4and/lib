#include "pch.h"

#include "Instance.h"
#include "Icon.h"

namespace wxutils {

#if BOOST_WINDOWS
wxIcon makeIcon(unsigned int resourceId, int sx, int sy)
{
    HICON hIcon = static_cast<HICON>(::LoadImage(instance(), MAKEINTRESOURCE(resourceId), IMAGE_ICON, sx, sy, 0));
    wxIcon icon;
    icon.SetHICON(hIcon);
    icon.SetSize(wxSize(sx, sy));
    
    return icon;
}
#endif

}
