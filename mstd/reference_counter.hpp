#pragma once

#include <cassert>
#include <boost/cstdint.hpp>

#include "config.hpp"

#if !defined(__S3E__)
#include "atomic.hpp"
#endif

#include "disposers.hpp"

namespace mstd {

template<typename T>
class atomic;

template<class T, class D, class C>
class reference_counter;

template<class Counter = atomic<size_t> >
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

    Counter references_;
    
    template<class T, class D, class C>
    friend void intrusive_ptr_add_ref(reference_counter<T, D, C> * obj);
    
    template<class T, class D, class C>
    friend void intrusive_ptr_release(reference_counter<T, D, C> * obj);
};

template<class T, class Releaser = delete_disposer, class Counter = atomic<size_t> >
class reference_counter : public reference_counter_base<Counter> {
};

template<class T, class D, class C>
inline void intrusive_ptr_add_ref(reference_counter<T, D, C> * obj)
{
    obj->add_ref();
}

template<class T, class D, class C>
inline void intrusive_ptr_release(reference_counter<T, D, C> * obj)
{
    if(obj->release())
        D()(static_cast<T*>(obj));
}

}
