/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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
