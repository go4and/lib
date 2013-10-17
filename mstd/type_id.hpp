/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace mstd {

template<class T>
class type_id {
public:
    static void * id()
    {
        static char id;
        return &id;
    }
};

}
