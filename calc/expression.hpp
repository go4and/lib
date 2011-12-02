#pragma once

#include "fwd.hpp"

namespace calc {

class parser {
public:
    parser()
        : input_(0)
    {
    }

    void parse(const std::wstring & inp, compiler & result);
private:
    void * input_;
};

}
