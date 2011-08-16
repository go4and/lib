#include "pch.h"

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
