/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include <boost/config.hpp>

#if !BOOST_WINDOWS
#include <stdio.h>
#endif

#include <sys/stat.h>

#include "filesystem.hpp"
#include "rc_buffer.hpp"

namespace mstd {

FILE * wfopen(const boost::filesystem::path & path, const char * mode)
{
#if BOOST_WINDOWS
    wchar_t buf[0x10];
    std::copy(mode, mode + strlen(mode) + 1, buf);
    return _wfopen(path.native().c_str(), buf);
#else
    return fopen(path.native().c_str(), mode);
#endif
}

FILE * wfopen_append(const boost::filesystem::path & path)
{
    FILE * result = mstd::wfopen(path, "rb+");
    if(!result)
        result = mstd::wfopen(path, "wb");
    return result;
}

std::streamsize file_size(FILE * file)
{
#if BOOST_WINDOWS
    struct _stat64 stat;
    if(!_fstat64(_fileno(file), &stat))
#else
    struct stat stat;
    if(!fstat(fileno(file), &stat))
#endif
        return stat.st_size;
    else
        return -1;
}

rc_buffer load_file(const boost::filesystem::wpath & path, bool addZero)
{
    FILE * file = wfopen(path, "rb");
    if(file)
    {
        off_t size = static_cast<off_t>(file_size(file));
        if(size >= 0)
        {
            rc_buffer result(size + (addZero ? 1 : 0));
            off_t read = fread(result.data(), 1, size, file);
            if(read == size)
            {
                if(addZero)
                    result.data()[size] = 0;
                fclose(file);
                return result;
            }
        }
        fclose(file);
    }

    return rc_buffer();
}

bool save_file(const boost::filesystem::wpath & path, const char * buf, size_t len, bool trimZero)
{
    bool result = false;
    FILE * file = wfopen(path, "wb");
    if(file)
    {
        off_t size = len;
        if(trimZero)
            size = std::find(buf, buf + size, 0) - buf;
        result = static_cast<off_t>(fwrite(buf, 1, size, file)) == size;
        fclose(file);
    }
    return result;
}

bool save_file(const boost::filesystem::wpath & path, const rc_buffer & data, bool trimZero)
{
    return save_file(path, data.data(), data.size(), trimZero);
}

namespace {

typedef boost::filesystem::path::string_type string_type;
typedef boost::filesystem::path::value_type char_type;

string_type get_env(const string_type & name)
{
#if BOOST_WINDOWS
    const wchar_t * env = _wgetenv(name.c_str());
#else
    const char * env = getenv(name.c_str());
#endif
    if(env)
        return env;
    else
        return string_type();
}

bool is_word(char_type ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_';
}

boost::filesystem::path do_expand_env_vars(const string_type & string)
{
    string_type result;
    string_type::const_iterator i = string.begin(), end = string.end();
    while(i != end)
    {
        char_type ch = *i;
        ++i;
        if(ch == '$')
        {
            if(i == end)
                break;
            else if(*i == '{')
            {
                ++i;
                string_type::const_iterator j = std::find(i, end, '}');
                if(j == end)
                    break;
                result += get_env(string_type(i, j));
                i = ++j;
            } else if(is_word(*i)) {
                string_type::const_iterator j = i;
                while(i != end && is_word(*i))
                    ++i;
                result += get_env(string_type(j, i));
            } else
                result += ch;
        } else if(ch == '\\')
        {
            if(i == end)
                break;
            else
                result += *i++;
        } else
            result += ch;
    }
    return boost::filesystem::path(result);
}

}

boost::filesystem::path expand_env_vars(const std::wstring & input)
{
#if BOOST_WINDOWS
    return do_expand_env_vars(input);
#else
    return do_expand_env_vars(utf8(input));
#endif
}

boost::filesystem::path expand_env_vars(const std::string & input)
{
#if BOOST_WINDOWS
    return do_expand_env_vars(deutf8(input));
#else
    return do_expand_env_vars(input);
#endif
}

}
