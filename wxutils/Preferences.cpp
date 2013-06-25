/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#<<<<<<< HEAD
include "pch.h"
>>>>>>> 751fc585e47bb4774a5a26110f78678513d85781
=======
#include "pch.h"
>>>>>>> 1f7bacbb2530b81519acd781a469dcdcefa85f68

#include "Preferences.h"

MLOG_DECLARE_LOGGER(wxutils_preferences);

namespace wxutils {

#if BOOST_WINDOWS
mstd::rc_buffer getRegValue(HKEY root, const wchar_t * path, const wchar_t * name)
{
    HKEY key;
    LSTATUS res = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_QUERY_VALUE, &key);
    if(res != ERROR_SUCCESS)
    {
        MLOG_ERROR("Failed to open key: " << res);
        return mstd::rc_buffer();
    }
    BOOST_SCOPE_EXIT(key) {
        RegCloseKey(key);
    } BOOST_SCOPE_EXIT_END;

    DWORD type;
    DWORD len = 0;
    res = RegQueryValueEx(key, name, 0, &type, 0, &len);

    if(res == ERROR_SUCCESS)
    {
        mstd::rc_buffer result(len);
        res = RegQueryValueEx(key, name, 0, &type, mstd::pointer_cast<BYTE*>(result.data()), &len);
        if(res == ERROR_SUCCESS)
        {
            result.resize(len);
            return result;
        } else
            MLOG_ERROR("Failed to query value: " << mstd::utf8(name) << ", " << res);
    } else
        MLOG_ERROR("Failed to query value size: " << mstd::utf8(name) << ", " << res);

    return mstd::rc_buffer();
}

bool setRegValue(HKEY root, const wchar_t * path, const wchar_t * name, const char * data, size_t size)
{
    HKEY key;
    LSTATUS res = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_SET_VALUE, &key);
    if(res != ERROR_SUCCESS)
    {
        if(res == ERROR_FILE_NOT_FOUND)
        {
            DWORD disp;            
            res = RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, 0, 0, KEY_SET_VALUE, 0, &key, &disp);
            if(res != ERROR_SUCCESS)
            {
                MLOG_ERROR("Failed to create key: " << res);
                return false;
            }
        } else {
            MLOG_ERROR("Failed to open key: " << res);
            return false;
        }
    }
    BOOST_SCOPE_EXIT(key) {
        RegCloseKey(key);
    } BOOST_SCOPE_EXIT_END;

    res = RegSetValueEx(key, name, 0, REG_BINARY, mstd::pointer_cast<const BYTE*>(data), size);

    if(res == ERROR_SUCCESS)
        return true;

    MLOG_ERROR("Failed to set value: " << mstd::utf8(name) << ", " << res);

    return false;
}
#endif

}
