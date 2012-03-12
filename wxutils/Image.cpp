#include "pch.h"

#ifndef __APPLE__

#include "Instance.h"

#include "CanvasUtils.h"
#include "Image.h"

namespace wxutils {

HBITMAP loadBitmap(unsigned int resourceId)
{
    if(resourceId)
        return LoadBitmap(instance(), MAKEINTRESOURCE(resourceId));
    else
        return NULL;
}

wxSize getSize(HBITMAP bmp)
{
    BITMAP info;
    GetObject(bmp, sizeof(info), &info);
    return wxSize(info.bmWidth, info.bmHeight);
}

Image::Image(unsigned int resourceId, unsigned int maskId)
    : bmp_(loadBitmap(resourceId)), mask_(loadBitmap(maskId)),
      dc_(CreateCompatibleDC(NULL)), size_(getSize(bmp_))
{
    SelectObject(dc_, bmp_);
}

Image::Image(HBITMAP bitmap, HBITMAP mask)
    : bmp_(bitmap), mask_(mask), dc_(CreateCompatibleDC(NULL)), size_(getSize(bmp_))
{
    SelectObject(dc_, bmp_);
}

Image::~Image()
{
    if(mask_)
        DeleteObject(mask_);
    if(bmp_)
        DeleteObject(bmp_);
    DeleteDC(dc_);
}

HBITMAP Image::getBitmap() const
{
    return bmp_;
}

HBITMAP Image::getMask() const
{
    return mask_;
}

HDC Image::getDc() const
{
    return dc_;
}

const wxSize & Image::size() const
{
    return size_;
}

void drawMasked(const Image & image, const wxPoint & p, wxDC & canvas, const wxRect & rect)
{
    HDC dc = static_cast<HDC>(canvas.GetHDC());
    MaskBlt(dc, rect.x, rect.y, rect.width, rect.height, image.getDc(), p.x, p.y, image.getMask(), p.x, p.y, MAKEROP4(BLACKNESS, SRCINVERT));
    MaskBlt(dc, rect.x, rect.y, rect.width, rect.height, image.getDc(), p.x, p.y, image.getMask(), p.x, p.y, MAKEROP4(SRCCOPY, SRCINVERT));
}

void draw(wxDC & canvas, int x, int y, int w, int h, const Image & image, int ox, int oy)
{
    BitBlt(static_cast<HDC>(canvas.GetHDC()), x, y, w, h, image.getDc(), ox, oy, SRCCOPY);
}

void tile(wxDC & canvas, long x, long y, long xs, long ys, const wxutils::Image & image, long ix, long iy, long sx, long sy)
{
    tile(canvas, x, y, xs, ys, image.getDc(), ix, iy, sx, sy);
}

void lighterImage(wxImage & img, int value)
{
    unsigned char * data = img.GetData();
    for(unsigned char * i = data, * end = data + img.GetWidth() * img.GetHeight() * 3; i != end; ++i)
        *i = std::min(0xff, static_cast<int>(*i) + value);
}

}

#endif