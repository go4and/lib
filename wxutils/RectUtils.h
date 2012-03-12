#pragma once

#if !defined(BUILDING_WXUTILS)
#include <vector>
#endif

#include "Config.h"

class wxRect;
class wxPoint;
struct tagRECT;
typedef tagRECT RECT;

namespace wxutils {

WXUTILS_DECL wxRect intersect(const wxRect & lhs, const wxRect & rhs);
WXUTILS_DECL bool contains(const wxRect & rect, const wxPoint & point);
WXUTILS_DECL bool contains(int n, const wxPoint * points, const wxPoint & point);
WXUTILS_DECL bool contains(const std::vector<wxPoint> & points, const wxPoint & point);
WXUTILS_DECL RECT toRect(const wxRect & rect);
WXUTILS_DECL void order(wxPoint & lt, wxPoint & rb);

}
