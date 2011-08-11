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
