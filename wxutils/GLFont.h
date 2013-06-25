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
private:
    GLuint makeGlyph(wchar_t ch);

    boost::unordered_map<wchar_t, GLuint> glyphs_;
    std::vector<GLuint> buffer_;
    wxMemoryDC mdc_;
};

}
