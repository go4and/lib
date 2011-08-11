#include "pch.h"

#include "Handler.h"

namespace nexus {

void HandlerStorageBase::allocationFailed(size_t size, size_t bufferSize)
{
    std::cerr << "Allocation failed, requested: " << size << ", buffer size: " << bufferSize << std::endl;
}

}
