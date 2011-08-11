#pragma once

namespace boost {
    template<class T> class intrusive_ptr;
}

namespace mstd {

class buffer;
typedef boost::intrusive_ptr<buffer> pbuffer;

}
