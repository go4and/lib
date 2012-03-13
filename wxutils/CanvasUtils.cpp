#include "pch.h"

#include "CanvasUtils.h"

namespace wxutils {

wxPoint getTextSize(wxDC & canvas, const wchar_t * str, size_t len)
{
#if BOOST_WINDOWS
	RECT rect = {0, 0, 0, 0};
	DrawText(static_cast<HDC>(canvas.GetHDC()), str, static_cast<int>(len), &rect, DT_CALCRECT);
	return wxPoint(rect.right, rect.bottom);
#else
    wxSize sz = canvas.GetTextExtent(wxString(str, len));
    return wxPoint(sz.x, sz.y);
#endif
}

wxPoint getTextSize(wxDC & canvas, const std::wstring & str)
{
    return getTextSize(canvas, str.c_str(), str.length());
}

wxPoint getTextSize(wxDC & canvas, const wchar_t * str)
{
    return getTextSize(canvas, str, wcslen(str));
}

wxPoint getTextSize(wxDC & canvas, const wxString & str)
{
    return getTextSize(canvas, str.c_str(), str.length());
}

long getTextHeight(wxDC & canvas)
{
    return getTextSize(canvas, L" ").y - 6;
}

#if BOOST_WINDOWS
void tile(wxDC & canvas, long x, long y, long xs, long ys, void * src, long ix, long iy, long sx, long sy)
{
    if(sx <= 0 || sy <= 0)
        return;
    long bottom = y + ys;
    long right = x + xs;
    while(y < bottom)
    {
        long cy = std::min(bottom - y, sy);
        long px = x;
        while(px < right)
        {
            long cx = std::min(right - px, sx);
            BitBlt(static_cast<HDC>(canvas.GetHDC()), px, y, cx, cy, static_cast<HDC>(src), ix, iy, SRCCOPY);
            px += cx;
        }
        y += cy;
    }
}

void drawText(wxDC & canvas, const wchar_t * str, size_t len, const wxRect & r, unsigned int format)
{
	RECT rc = { r.x, r.y, r.x + r.width, r.y + r.height };
	DrawText(static_cast<HDC>(canvas.GetHDC()), str, static_cast<int>(len), &rc, format );
}

void drawText(wxDC & canvas, const wchar_t * str, const wxRect & r, unsigned int format)
{
    drawText(canvas, str, wcslen(str), r, format);
}

void drawText(wxDC & canvas, const std::wstring & str, const wxRect & r, unsigned int format)
{
    drawText(canvas, str.c_str(), str.length(), r, format);
}

void drawText(wxDC & canvas, const wxString & str, const wxRect & r, unsigned int format)
{
    drawText(canvas, str.c_str(), str.length(), r, format);
}

ClipRect::ClipRect(wxDC & canvas, const wxRect & rect)
    : dc_(canvas.GetHDC())
{
    const RECT temp = { rect.x, rect.y, rect.x + rect.width, rect.y + rect.height };
    rgn_ = CreateRectRgnIndirect(&temp);
    SelectClipRgn(static_cast<HDC>(dc_), static_cast<HRGN>(rgn_));
}

ClipRect::~ClipRect()
{
    SelectClipRgn(static_cast<HDC>(dc_), NULL);
    DeleteObject(static_cast<HRGN>(rgn_));
}
#endif

}
