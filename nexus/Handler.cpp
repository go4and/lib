/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Handler.h"

namespace nexus {

void HandlerStorageBase::allocationFailed(size_t size, size_t bufferSize)
{
    std::cerr << "Allocation failed, requested: " << size << ", buffer size: " << bufferSize << std::endl;
}

}
