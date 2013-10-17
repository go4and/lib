/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include "Config.h"

extern "C" {
    struct HINSTANCE__;
    typedef HINSTANCE__ *HINSTANCE;
}

namespace wxutils {

WXUTILS_DECL HINSTANCE instance();
bool checkSingleInstance(bool show, const std::wstring & title = std::wstring());
void releaseSingleInstance();

}
