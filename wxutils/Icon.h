/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include "Config.h"

class wxIcon;

namespace wxutils {

WXUTILS_DECL wxIcon makeIcon(unsigned int resourceId, int sx, int sy = -1);
WXUTILS_DECL void addIcon(wxIconBundle & icons, unsigned int resourceId, int sx, int sy = -1);

}
