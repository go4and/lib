/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace mnet {

struct CurlMultiTraits {
    static CURLM * null()
    {
        return 0;
    }

    static void close(CURLM * handle)
    {
        curl_multi_cleanup(handle);
    }
};

typedef mstd::handle_base<CURLM*, mstd::comparable_traits<CurlMultiTraits> > CurlMultiHandle;

size_t appendToString(const void *ptr, size_t size, size_t nmemb, void* stream);

}
