/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef BUILDING_WXUTILS
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

#include <wx/dcmemory.h>
#endif

namespace wxutils {

class GLFont : boost::noncopyable {
public:
    explicit GLFont(const wxFont & font);
    ~GLFont();

    void draw(const wchar_t * str, size_t len);
    void draw(const wchar_t * str);
    inline void draw(const std::wstring & str) { draw(str.c_str(), str.length()); }
    inline void draw(const wxString & str) { draw(str.c_str().AsWChar(), str.length()); }
<<<<<<< HEAD
=======

    wxSize textSize(const wxString & str) { return mdc_.GetTextExtent(str); }
>>>>>>> 751fc585e47bb4774a5a26110f78678513d85781
private:
    GLuint makeGlyph(wchar_t ch);

    boost::unordered_map<wchar_t, GLuint> glyphs_;
    std::vector<GLuint> buffer_;
    wxMemoryDC mdc_;
};

}
