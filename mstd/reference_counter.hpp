#pragma once

#include <boost/cstdint.hpp>

#include "config.hpp"

#include "atomic.hpp"

namespace mstd {

template<class T, class D>
class reference_counter;

class MSTD_DECL reference_counter_base {
public:
    reference_counter_base()
        : references_(0) {}

    size_t get_current_number_of_references()
    {
        return references_;
    }
private:
    void add_ref()
    {
        references_++;
    }

    bool release()
    {
        size_t value = references_--;
        BOOST_ASSERT(value <= 0x10000);
        return value == 1;
    }

    atomic<size_t> references_;
    
    template<class T, class D>
    friend void intrusive_ptr_add_ref(reference_counter<T, D> * obj);
    
    template<class T, class D>
    friend void intrusive_ptr_release(reference_counter<T, D> * obj);
};

struct Deleter {
    template<class T>
    static void release(T * t)
    {
        delete t;
    }
};

template<class T, class Releaser = Deleter>
class reference_counter : public reference_counter_base {
};

template<class T, class D>
inline void intrusive_ptr_add_ref(reference_counter<T, D> * obj)
{
    obj->add_ref();
}

template<class T, class D>
inline void intrusive_ptr_release(reference_counter<T, D> * obj)
{
    if(obj->release())
        D::release(static_cast<T*>(obj));
}

}
