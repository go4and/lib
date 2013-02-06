#pragma once

#if !defined(BUILDING_WXUTILS)
#include <string>

#include <boost/noncopyable.hpp>

#include <wx/dc.h>
#endif

#include "Config.h"

class wxDC;
class wxPoint;
class wxRect;
class wxString;

namespace wxutils {

class Image;

WXUTILS_DECL wxPoint getTextSize(HDC dc, const wchar_t * str, size_t len);
inline wxPoint getTextSize(HDC dc, const wchar_t * str) { return getTextSize(dc, str, wcslen(str)); }
inline wxPoint getTextSize(HDC dc, const std::wstring & str) { return getTextSize(dc, str.c_str(), str.length()); }
inline wxPoint getTextSize(HDC dc, const wxString & str) { return getTextSize(dc, str.c_str(), str.length()); }
inline wxPoint getTextSize(wxDC & canvas, const wchar_t * str, size_t len) { return getTextSize(static_cast<HDC>(canvas.GetHDC()), str, len); }
inline wxPoint getTextSize(wxDC & canvas, const wchar_t * str) { return getTextSize(canvas, str, wcslen(str)); }
inline wxPoint getTextSize(wxDC & canvas, const std::wstring & str) { return getTextSize(canvas, str.c_str(), str.length()); }
inline wxPoint getTextSize(wxDC & canvas, const wxString & str) { return getTextSize(canvas, str.c_str(), str.length()); }
WXUTILS_DECL long getTextHeight(wxDC & canvas);
WXUTILS_DECL void tile(wxDC & canvas, long x, long y, long xs, long ys, void * src, long ix, long iy, long sx, long sy);

void drawText(HDC dc, const wchar_t * str, size_t len, const wxRect & r, unsigned int format);
inline void drawText(HDC dc, const std::wstring & str, const wxRect & r, unsigned int format) { drawText(dc, str.c_str(), str.length(), r, format); }
inline void drawText(HDC dc, const wxString & str, const wxRect & r, unsigned int format) { drawText(dc, str.c_str(), str.length(), r, format); }
inline void drawText(HDC dc, const wchar_t * str, const wxRect & r, unsigned int format) { drawText(dc, str, wcslen(str), r, format); }

inline void drawText(wxDC & canvas, const wchar_t * str, size_t len, const wxRect & r, unsigned int format) { drawText(static_cast<HDC>(canvas.GetHDC()), str, len, r, format); }
inline void drawText(wxDC & canvas, const std::wstring & str, const wxRect & r, unsigned int format) { drawText(canvas, str.c_str(), str.length(), r, format); }
inline void drawText(wxDC & canvas, const wxString & str, const wxRect & r, unsigned int format) { drawText(canvas, str.c_str(), str.length(), r, format); }
inline void drawText(wxDC & canvas, const wchar_t * str, const wxRect & r, unsigned int format) { drawText(canvas, str, wcslen(str), r, format); }

class WXUTILS_DECL ClipRect : private boost::noncopyable {
public:
    ClipRect(wxDC & canvas, const wxRect & rect);
    ~ClipRect();
private:
    void * dc_;
    void * rgn_;
};

}
