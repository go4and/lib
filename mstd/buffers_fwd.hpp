/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace boost {
    template<class T> class intrusive_ptr;
}

namespace mstd {

class buffer;
typedef boost::intrusive_ptr<buffer> pbuffer;

}
