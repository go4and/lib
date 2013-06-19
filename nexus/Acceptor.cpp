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
