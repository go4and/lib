/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "GZip.h"

namespace marc {

#if BOOST_WINDOWS && !defined(LIBARCHIVE_STATIC)
    typedef int ZEXPORT (*gzreadType)(gzFile file, voidp buf, unsigned len);
    typedef const char * ZEXPORT (*gzerrorType)(gzFile file, int *errnum);
    typedef int ZEXPORT (*gzcloseType)(gzFile file);
    typedef gzFile ZEXPORT (*gzopenType)(const char *, const char *);
    typedef int ZEXPORT (*inflateType)(z_streamp strm, int flush);
    typedef int ZEXPORT (*inflateEndType)(z_streamp strm);
    typedef int ZEXPORT (*inflateInit2_Type)(z_streamp strm, int  windowBits, const char *version, int stream_size);

#define ZLIB_FUNCTIONS (gzread)(gzerror)(gzclose)(gzopen)(inflate)(inflateEnd)(inflateInit2_)

#define ZLIB_FUNCTIONS_VAR(s, data, elem) BOOST_PP_CAT(elem, Type) BOOST_PP_CAT(m_, elem);
    BOOST_PP_SEQ_FOR_EACH(ZLIB_FUNCTIONS_VAR, ~, ZLIB_FUNCTIONS);

    class GZipInit {
        MSTD_SINGLETON_INLINE_DEFINITION(GZipInit);
    public:
        ~GZipInit()
        {
            FreeLibrary(zlib_);
        }
    private:
#define ZLIB_FUNCTIONS_GPA(s, data, elem) BOOST_PP_CAT(m_, elem) = reinterpret_cast<BOOST_PP_CAT(elem, Type)>(GetProcAddress(zlib_, BOOST_PP_STRINGIZE(elem)));

        GZipInit()
        {
            zlib_ = LoadLibrary(L"zlib1.dll");
            BOOST_PP_SEQ_FOR_EACH(ZLIB_FUNCTIONS_GPA, ~, ZLIB_FUNCTIONS);
            m_gzread = reinterpret_cast<gzreadType>(GetProcAddress(zlib_, "gzread"));
        }

        HMODULE zlib_;
    };
#else
#define m_gzread gzread
#define m_gzerror gzerror
#define m_gzclose gzclose
#define m_gzopen gzopen
#define m_inflate inflate
#define m_inflateEnd inflateEnd
#define m_inflateInit2_ inflateInit2_

class GZipInit { public: static void instance() {} };
#endif

void ungzip(const void * data, size_t len, std::vector<char> & out)
{
    GZipInit::instance();
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.next_in = static_cast<Bytef*>(const_cast<void*>(data));
    stream.avail_in = len;

    out.resize(len);
    out.resize(out.capacity());
    stream.next_out = static_cast<Bytef*>(static_cast<void*>(&out[0]));
    stream.avail_out = out.size();

    if(m_inflateInit2_(&stream, 32 + 15, ZLIB_VERSION, sizeof(z_stream)) == Z_OK)
    {
        int res;
        while((res = m_inflate(&stream, Z_NO_FLUSH)) == Z_OK)
        {
            ptrdiff_t offset = stream.next_out - static_cast<Bytef*>(static_cast<void*>(&out[0]));
            out.resize(out.size() * 2);
            out.resize(out.capacity());
            stream.next_out = static_cast<Bytef*>(static_cast<void*>(&out[offset]));
            stream.avail_out = out.size() - offset;
        }
        out.resize(stream.total_out);
        m_inflateEnd(&stream);
    }
}

}
