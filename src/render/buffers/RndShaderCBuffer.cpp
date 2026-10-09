#include "render/buffers/RndShaderCBuffer.h"

#include "render/resources/shaders/shader_constant_block.h"
#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x639F30.
RndShaderCBuffer* RndShaderCBuffer::New(
    const RndShaderCBufferConfig& config,
    unsigned int flags,
    unsigned long numElements) {
    if (numElements == kConfigElementCount) {
        numElements = config.next_offset;
    }

    auto* buffer =
        TheRndFactory()->CreateShaderCBuffer(config, flags, numElements);
    if ((flags & kDeferInitialSync) == 0 && buffer->mSyncPending) {
        buffer->_CreateImpl();
        buffer->mSyncPending = false;
    }
    return buffer;
}

// Reconstructed from eboot.elf at 0x639FF0.
RndShaderCBuffer::RndShaderCBuffer(
    const RndShaderCBufferConfig& config,
    unsigned int flags,
    unsigned long numElements,
    void* data)
    : mName(config.name),
      mFlags(flags),
      mIndex(config.buffer_index),
      mStageMask(config.stage_mask),
      mConfigNumElements(config.next_offset),
      mNumElements(numElements),
      mData(data),
      mSyncPending(true) {}

void RndShaderCBuffer::SafeDelete(RndShaderCBuffer*& buffer) {
    if (buffer != nullptr) {
        buffer->~RndShaderCBuffer();
        MemFree(buffer);
        buffer = nullptr;
    }
}
