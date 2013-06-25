/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "pch.h"

#include "Manager.h"
#include "Utils.h"

#if !MLOG_NO_LOGGING

namespace mlog {

template<class Ch, class CT>
void outNow(std::basic_ostream<Ch, CT> & out)
{
    Ch buffer[16];
    buffer[2] = ' ';
    buffer[5] = ':';
    buffer[8] = ':';
    buffer[11] = '.';
    buffer[15] = 0;

#if defined(BOOST_WINDOWS)
    SYSTEMTIME st;
    GetLocalTime(&st);

    buffer[ 0] = '0' + (st.wDay / 10);
    buffer[ 1] = '0' + (st.wDay % 10);
    buffer[ 3] = '0' + (st.wHour / 10);
    buffer[ 4] = '0' + (st.wHour % 10);
    buffer[ 6] = '0' + (st.wMinute / 10);
    buffer[ 7] = '0' + (st.wMinute % 10);
    buffer[ 9] = '0' + (st.wSecond / 10);
    buffer[10] = '0' + (st.wSecond % 10);
    buffer[12] = '0' + (st.wMilliseconds / 100);
    buffer[13] = '0' + ((st.wMilliseconds / 10) % 10);
    buffer[14] = '0' + (st.wMilliseconds % 10);
#else
    timeval tv;
    gettimeofday(&tv, 0);
    tm tm;
    localtime_r(&tv.tv_sec, &tm);

    buffer[ 0] = '0' + (tm.tm_mday / 10);
    buffer[ 1] = '0' + (tm.tm_mday % 10);
    buffer[ 3] = '0' + (tm.tm_hour / 10);
    buffer[ 4] = '0' + (tm.tm_hour % 10);
    buffer[ 6] = '0' + (tm.tm_min / 10);
    buffer[ 7] = '0' + (tm.tm_min % 10);
    buffer[ 9] = '0' + (tm.tm_sec / 10);
    buffer[10] = '0' + (tm.tm_sec % 10);
    buffer[12] = '0' + (tv.tv_usec / 100000);
    buffer[13] = '0' + ((tv.tv_usec / 10000) % 10);
    buffer[14] = '0' + ((tv.tv_usec / 1000) % 10);
#endif
    out << buffer;
}

void OutNow::operator ()(std::ostream & out) const
{
    outNow(out);
}

void OutNow::operator ()(std::wostream & out) const
{
    outNow(out);
}

bool setup(const char * variable, const char * appname)
{
    return setupFromFile(getenv(variable), appname);
}
    
bool setupFromFile(const char * filename, const char * appname)
{
    Manager & man = Manager::instance();
    man.setAppName(appname);
    if(filename)
    {
        std::ifstream inp(filename);
        if(inp)
        {
            bool match = true;
            std::string line;
            while(std::getline(inp, line))
            {
                if(line.empty() || line[0] == '#')
                    continue;
                if(line[0] == '[' && line[line.size() - 1] == ']')
                    match = line.size() == 2 || (line.substr(1, line.size() - 2) == appname);
                else if(match)
                    try {
                        man.setup(line);
                    } catch(std::exception &) {
                    }
            }
            return true;
        }
    }
    return false;
}

}

#endif
