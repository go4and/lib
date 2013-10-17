/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Error.h"

#include "EC.h"

namespace mcrypt {

namespace {

int ecg2nid[] = { NID_sect283k1 };

EC doGenerate(EC_GROUP * group, Error & error)
{
    BOOST_SCOPE_EXIT((&group)) {
        if(group)
            EC_GROUP_free(group);
    } BOOST_SCOPE_EXIT_END;

    EC_KEY * key = EC_KEY_new();
    if(!key && error.checkResult(0))
        return EC(0);
    BOOST_SCOPE_EXIT((&key)) {
        if(key)
            EC_KEY_free(key);
    } BOOST_SCOPE_EXIT_END;
    
    if(error.checkResult(EC_KEY_set_group(key, group)))
        return EC(0);
    group = 0;

    if(error.checkResult(EC_KEY_generate_key(key)))
        return EC(0);

    EVP_PKEY * pkey = EVP_PKEY_new();
    EVP_PKEY_assign_EC_KEY(pkey, key);
    key = 0;
    return EC(pkey);
}

}

EC EC::generate(ECGroup ecg, Error & error)
{
    int nid = ecg2nid[ecg];

    EC_GROUP * group = EC_GROUP_new_by_curve_name(nid);
    if(!group && error.checkResult(0))
        return EC(0);

    return doGenerate(group, error);
}

EC EC::generate(const EC & ec, Error & error)
{
    EVP_PKEY * pkey = static_cast<EVP_PKEY*>(ec.handle());
    EC_KEY * eckey = EVP_PKEY_get1_EC_KEY(pkey);
    if(!eckey && error.checkResult(0))
        return EC(0);
    EC_GROUP * group = EC_GROUP_dup(EC_KEY_get0_group(eckey));

    return doGenerate(group, error);
}

}
