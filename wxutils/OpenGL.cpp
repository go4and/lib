/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "pch.h"

#include "OpenGL.h"

MLOG_DECLARE_LOGGER(wxutils_opengl);

namespace wxutils {

class CheckHasNP2 {
    MSTD_SINGLETON_INLINE_DEFINITION(CheckHasNP2);
public:
    bool value() const
    {
        return value_;
    }
private:
    CheckHasNP2()
    {
        std::string ext = mstd::pointer_cast<const char*>(glGetString(GL_EXTENSIONS));
        value_ = ext.find("GL_ARB_texture_non_power_of_two") != std::string::npos;
    }

    bool value_;
};

GLBitmap::GLBitmap()
    : width_(0), height_(0), data_(0) {}

GLBitmap::GLBitmap(GLuint width, GLuint height, GLubyte * data)
    : width_(width), height_(height), data_(data)
{
    addRef();
}

GLBitmap::GLBitmap(const GLBitmap & rhs)
    : width_(rhs.width_), height_(rhs.height_), data_(rhs.data_)
{
    addRef();
}

void GLBitmap::operator=(const GLBitmap & rhs)
{
    removeRef();
    width_ = rhs.width_;
    height_ = rhs.height_;
    data_ = rhs.data_;
    addRef();
}

void GLBitmap::removeRef()
{
    if(data_ && !--*mstd::pointer_cast<boost::uint32_t*>(data_))
        delete [] data_;
}

void GLBitmap::addRef()
{
    if(data_)
        ++*mstd::pointer_cast<boost::uint32_t*>(data_);
}

GLBitmap::~GLBitmap()
{
    removeRef();
}

void fillImageData(wxImage & img, unsigned char * output, size_t owidth, size_t oheight, ImageFlags flags)
{
    if(!img.HasAlpha() && img.HasMask())
        img.InitAlpha();
    unsigned char * idata = img.GetData();
    unsigned char * alpha = img.GetAlpha();
    size_t height = img.GetHeight();
    size_t width = img.GetWidth();
    for(size_t y = 0; y != height; ++y)
    {
        unsigned char * inpbase = idata + y * width * 3;
        unsigned char * alphabase = alpha + y * width;
        unsigned char * outbase = output + y * owidth * 4;
        for(size_t x = 0; x != width; ++x)
        {
            unsigned char * out = outbase + x * 4;
            unsigned char * inp = inpbase + x * 3;
            if(flags.has<ifGrayscale>()) {
                unsigned short sum = inp[0];
                sum += inp[1];
                sum += inp[2];
                if(flags.has<ifHighlight>())
                    sum += 0x60;
                memset(out, sum / 3, 3);
            } else if(flags.has<ifHighlight>()) {
                out[0] = std::min(inp[0], static_cast<unsigned char>(0xdf)) + 0x20;
                out[1] = std::min(inp[1], static_cast<unsigned char>(0xdf)) + 0x20;
                out[2] = std::min(inp[2], static_cast<unsigned char>(0xdf)) + 0x20;
            } else {
                out[0] = inp[0];
                out[1] = inp[1];
                out[2] = inp[2];
            }
            out[3] = alpha ? alphabase[x] : 0xff;
        }
    }
}

void GLTexture::reset()
{
    if(id_)
    {
        MLOG_DEBUG("Release texture: " << id_);
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}

GLTexture loadTexture(const wxImage & simg, ImageFlags flags)
{
    MLOG_DEBUG("loadTexture(" << simg.GetWidth() << ", " << simg.GetHeight() << ")");

    GLint texSize = 0; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);

    wxImage img;
    if(simg.GetWidth() > texSize || simg.GetHeight() > texSize)
    {
        double ratio = std::min(texSize * 1.0 / simg.GetWidth(), texSize * 1.0 / simg.GetHeight());
        img = simg.Scale(std::min<int>(texSize, simg.GetWidth() * ratio), std::min<int>(texSize, simg.GetHeight() * texSize));
    } else
        img = simg;

    GLuint texture;
    glGenTextures(1, &texture);

    int width = img.GetWidth();
    int height = img.GetHeight();
    int twidth = width;
    int theight = height;
    if(!CheckHasNP2::instance().value())
    {
        twidth = 1;
        while(twidth < width)
            twidth += twidth;
        theight = 1;
        while(theight < height)
            theight += theight;
    }

    std::vector<unsigned char> data(twidth * theight * 4);
    fillImageData(img, &data[0], twidth, theight, flags);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, twidth, theight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    GLenum error = glGetError();
    if(error)
        wxMessageBox(str(boost::wformat(L"Error setting texture: %1%.") % error).c_str(), L"Error!!!", wxICON_ERROR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return GLTexture(width, height, static_cast<double>(width) / twidth, static_cast<double>(height) / theight, texture);
}

GLBitmap loadBitmap(wxImage & img, ImageFlags flags)
{
    GLuint width = img.GetWidth();
    GLuint height = img.GetHeight();
    GLubyte * data = new unsigned char[4 + width * height * 4];
    memset(data, 0, 4);
    fillImageData(img, 4 + data, width, height, flags);
    return GLBitmap(width, height, data);
}

class SinCos : public mstd::singleton<SinCos> {
public:
    void get(int steps, double & si, double & co)
    {
        Cache::const_iterator i = cache_.find(steps);
        if(i == cache_.end())
            i = cache_.insert(Cache::value_type(steps, std::make_pair(sin(2 * M_PI / steps), cos(2 * M_PI / steps)))).first;
        si = i->second.first;
        co = i->second.second;
    }
private:
    SinCos()
    {
    }

    typedef boost::unordered_map<int, std::pair<double, double> > Cache;
    Cache cache_;

    MSTD_SINGLETON_DECLARATION(SinCos);
};

void glEllipse(double x, double y, double rx, double ry, double sx, double sy, bool fill)
{
    double max = std::max(rx / sx, ry / sy);
    int circleSteps = max <= 4 ? 4 : 32;
    double si;
    double co;
    SinCos::instance().get(circleSteps, si, co);

    glBegin(fill ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
        if(fill)
        {
            glVertex2d(x, y);
            glVertex2d(x + rx, y);
        }
        double ox = 1.0;
        double oy = 0.0;
        for(int i = 0; i != circleSteps; ++i)
        {
            double n = ox * co - oy * si;
            oy = ox * si + oy * co;
            ox = n;
            glVertex2d(x + rx * ox, y + ry * oy);
        }
    glEnd();
}

void glBoundedEllipse(double x1, double y1, double x2, double y2, double sx, double sy, bool fill)
{
    glEllipse((x1 + x2) * 0.5, (y1 + y2) * 0.5, (x2 - x1) * 0.5, (y2 - y1) * 0.5, sx, sy, fill);
}

}
