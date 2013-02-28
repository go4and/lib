#pragma once

#if !defined(BUILDING_WXUTILS)
#include <boost/mpl/int.hpp>
#include <boost/move/move.hpp>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <gl/gl.h>
#endif

#include <mstd/enum_set.hpp>
#endif

#include "Config.h"

class wxImage;

namespace wxutils {

class ImageFlagsTag : public boost::mpl::int_<2> {};
typedef mstd::enum_set<ImageFlagsTag> ImageFlags;
typedef mstd::enum_set_item<ImageFlagsTag, 0> ifHighlight;
typedef mstd::enum_set_item<ImageFlagsTag, 1> ifGrayscale;

class WXUTILS_DECL GLBitmap {
public:
    GLBitmap();
    GLBitmap(GLuint width, GLuint height, GLubyte * data);

    GLBitmap(const GLBitmap & rhs);
    void operator=(const GLBitmap & rhs);

    ~GLBitmap();

    GLuint width() const
    {
        return width_;
    }

    GLuint height() const
    {
        return height_;
    }

    GLubyte * data() const
    {
        return data_ ? data_ + 4 : 0;
    }
private:
    void addRef();
    void removeRef();

    GLuint width_;
    GLuint height_;
    GLubyte * data_;
};

class WXUTILS_DECL GLTexture {
    BOOST_MOVABLE_BUT_NOT_COPYABLE(GLTexture);
public:
    GLTexture()
        : x_(0), y_(0), id_(0) {}

    GLTexture(double x, double y, GLuint id)
        : x_(x), y_(y), id_(id) {}

    ~GLTexture()
    {
        reset();
    }

    GLTexture(BOOST_RV_REF(GLTexture) texture)
        : x_(texture.x_), y_(texture.y_), id_(texture.id_)
    {
        texture.id_ = 0;
    }

    void operator=(BOOST_RV_REF(GLTexture) texture)
    {
        reset();
        x_ = texture.x_;
        y_ = texture.y_;
        id_ = texture.id_;
        texture.id_ = 0;
    }

    void reset();

    inline double x() const
    {
        return x_;
    }

    inline double y() const
    {
        return y_;
    }

    inline GLuint id() const
    {
        return id_;
    }

    inline bool operator!() const
    {
        return !id_;
    }
private:
    double x_;
    double y_;
    GLuint id_;
};

WXUTILS_DECL GLTexture loadTexture(const wxImage & image, ImageFlags flags);
WXUTILS_DECL GLBitmap loadBitmap(wxImage & image, ImageFlags flags);

WXUTILS_DECL void glEllipse(double x, double y, double rx, double ry, double sx, double sy, bool fill);
WXUTILS_DECL void glBoundedEllipse(double x1, double y1, double x2, double y2, double sx, double sy, bool fill);

}
