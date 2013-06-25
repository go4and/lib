/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Utils.h"

#include "Acceptor.h"

MLOG_DECLARE_LOGGER(nexus_acceptor);

namespace nexus {

#if !defined(MLOG_NO_LOGGING)
mlog::Logger & BaseAcceptor::getLogger()
{
    return logger;
}
#endif

}
