#pragma once

#include "Config.h"

class wxIcon;

namespace wxutils {

WXUTILS_DECL wxIcon makeIcon(unsigned int resourceId, int sx, int sy = -1);
WXUTILS_DECL void addIcon(wxIconBundle & icons, unsigned int resourceId, int sx, int sy = -1);

}
