#pragma once

#ifndef __APPLE__

#if !defined(BUILDING_WXUTILS)
#include <boost/noncopyable.hpp>

#include <wx/wx.h>
#endif

#include "Config.h"

namespace wxutils {

class WXUTILS_DECL Image : public boost::noncopyable {
public:
    explicit Image(unsigned int resourceId, unsigned int maskId);
    explicit Image(HBITMAP bitmap, HBITMAP mask = 0);
    ~Image();

    HBITMAP getMask() const;
    HBITMAP getBitmap() const;
    HDC getDc() const;
    const wxSize & size() const;
private:
    HBITMAP bmp_;
    HBITMAP mask_;
    HDC dc_;
    wxSize size_;
};

WXUTILS_DECL void drawMasked(const Image & image, const wxPoint & p, wxDC & canvas, const wxRect & rect);
WXUTILS_DECL void draw(wxDC & canvas, int x, int y, int w, int h, const Image & image, int ox, int oy);
WXUTILS_DECL void tile(wxDC & canvas, long x, long y, long xs, long ys, const wxutils::Image & image, long ix, long iy, long sx, long sy);
WXUTILS_DECL void lighterImage(wxImage & img, int value);

}

#endif
