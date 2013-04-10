#pragma once

#include "PKey.h"

namespace mcrypt {

enum ECGroup {
    ecg_sect283k1,
};

class EC : public GenericPKey {
    BOOST_MOVABLE_BUT_NOT_COPYABLE(EC);
public:
    static EC generate(ECGroup group, Error & error);
    static EC generate(const EC & group, Error & error);

    explicit EC(void * evp)
        : GenericPKey(evp)
    {
    }

    EC(BOOST_RV_REF(EC) rhs)
        : GenericPKey(boost::move(static_cast<GenericPKey&>(rhs)))
    {
    }
};

}
