/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "GLFont.h"

namespace wxutils {

GLFont::GLFont(const wxFont & font)
{
    mdc_.SetFont(font);
}

GLFont::~GLFont()
{
    for(auto i = glyphs_.begin(), end = glyphs_.end(); i != end; ++i)
        glDeleteLists(i->second, 1);
}

#if BOOST_WINDOWS

void GLFont::draw(const wchar_t * str)
{
    draw(str, wcslen(str));
}

GLuint GLFont::makeGlyph(wchar_t ch)
{
    GLuint result = glGenLists(1);
    BOOL res = wglUseFontBitmaps(static_cast<HDC>(mdc_.GetHDC()), ch, 1, result);
    BOOST_VERIFY(res);
    return result;
}

void GLFont::draw(const wchar_t * str, size_t len)
{
    buffer_.resize(len);
    for(const wchar_t * i = str, * end = str + len; i != end; ++i)
    {
        auto j = glyphs_.find(*i);
        if(j == glyphs_.end())
            j = glyphs_.insert(std::make_pair(*i, makeGlyph(*i))).first;
        buffer_[i - str] = j->second;
    }
    glCallLists(len, GL_UNSIGNED_INT, &buffer_[0]);
}

#endif

}
