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
