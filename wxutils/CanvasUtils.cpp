/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "CanvasUtils.h"

namespace wxutils {

#if BOOST_WINDOWS
wxPoint getTextSize(HDC dc, const wchar_t * str, size_t len)
{
	RECT rect = {0, 0, 0, 0};
	DrawText(dc, str, static_cast<int>(len), &rect, DT_CALCRECT);
	return wxPoint(rect.right, rect.bottom);
}

long getTextHeight(wxDC & canvas)
{
    return getTextSize(canvas, L" ").y - 2;
}
#endif

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

void drawText(HDC dc, const wchar_t * str, size_t len, const wxRect & r, unsigned int format)
{
	RECT rc = { r.x, r.y, r.x + r.width, r.y + r.height };
	DrawText(dc, str, static_cast<int>(len), &rc, format );
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
