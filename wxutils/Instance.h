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
