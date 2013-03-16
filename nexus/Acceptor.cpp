#include "pch.h"

#include "Utils.h"

#include "Acceptor.h"

MLOG_DECLARE_LOGGER(nexus_acceptor);

namespace nexus {

mlog::Logger & BaseAcceptor::getLogger()
{
    return logger;
}

}
