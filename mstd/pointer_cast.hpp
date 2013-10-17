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

template<class Out, class In>
inline Out pointer_cast(In * ptr)
{
    return static_cast<Out>(static_cast<void*>(ptr));
}

template<class Out, class In>
inline Out pointer_cast(const In * ptr)
{
    return static_cast<Out>(static_cast<const void*>(ptr));
}

}
