#pragma once

#if !defined(BUILDING_WXUTILS)
#include <string>

#include <boost/noncopyable.hpp>
#endif

#include "Config.h"

class wxDC;
class wxPoint;
class wxRect;
class wxString;

namespace wxutils {

class Image;

WXUTILS_DECL wxPoint getTextSize(wxDC & canvas, const wchar_t * str);
WXUTILS_DECL wxPoint getTextSize(wxDC & canvas, const std::wstring & str);
WXUTILS_DECL wxPoint getTextSize(wxDC & canvas, const wxString & str);
WXUTILS_DECL long getTextHeight(wxDC & canvas);
WXUTILS_DECL void tile(wxDC & canvas, long x, long y, long xs, long ys, void * src, long ix, long iy, long sx, long sy);

WXUTILS_DECL void drawText(wxDC & canvas, const std::wstring & str, const wxRect & r, unsigned int format);
WXUTILS_DECL void drawText(wxDC & canvas, const wxString & str, const wxRect & r, unsigned int format);
WXUTILS_DECL void drawText(wxDC & canvas, const wchar_t * str, const wxRect & r, unsigned int format);
WXUTILS_DECL void drawText(wxDC & canvas, const wchar_t * str, size_t len, const wxRect & r, unsigned int format);

class WXUTILS_DECL ClipRect : private boost::noncopyable {
public:
    ClipRect(wxDC & canvas, const wxRect & rect);
    ~ClipRect();
private:
    void * dc_;
    void * rgn_;
};

}
