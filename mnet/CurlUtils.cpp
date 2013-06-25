/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "pch.h"

#include "CurlUtils.h"

namespace mnet {

size_t appendToString(const void *ptr, size_t size, size_t nmemb, void* stream)
{
    std::string* ps = static_cast<std::string*>(stream);
    size *= nmemb;
    if(size)  
        ps->append(static_cast<const char*>(ptr), size);
    return size;
}  

}
