#include "pch.h"

#include "Instance.h"

#if BOOST_WINDOWS
extern "C"
{
    WXDLLIMPEXP_BASE HINSTANCE wxGetInstance();
}

namespace wxutils {

HINSTANCE instance()
{
    return wxGetInstance();
}

}
#endif