#include "render/buffers/RndComputeBuffer.h"

#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x636C70.
RndComputeBuffer* RndComputeBuffer::New(const Description& desc) {
    auto* buffer = TheRndFactory()->CreateComputeBuffer(desc);
    const auto staging_size =
        buffer->mDesc.mNumElements * buffer->mDesc.mElementSize;
    buffer->mStagingData = operator new(staging_size);
    buffer->_SyncStaticImpl();
    return buffer;
}

// Reconstructed from eboot.elf at 0x636CC0.
RndComputeBuffer::RndComputeBuffer(const Description& desc)
    : mDesc(desc), mStagingData(nullptr) {}

// Reconstructed from eboot.elf at 0x636D10. The deleting destructor at
// 0x636D50 releases the buffer through MemFree.
RndComputeBuffer::~RndComputeBuffer() {
    if (mStagingData != nullptr) {
        MemFree(mStagingData);
    }
}

// Reconstructed from eboot.elf at 0x636DB0.
int RndComputeBuffer::_GetTypeImpl() const {
    return -1;
}
