#include "renderps4/buffers/PS4ComputeBuffer.h"

#include <cstring>

// Reconstructed from eboot.elf at 0x8E3250.
PS4ComputeBuffer::PS4ComputeBuffer(const Description& desc)
    : RndComputeBuffer(desc),
      mBuffers{},
      mStorage{nullptr, nullptr},
      mActiveBank(0) {}

// Reconstructed from eboot.elf at 0x8E3290. The deleting destructor is at
// 0x8E32F0.
PS4ComputeBuffer::~PS4ComputeBuffer() {
    _FreeImpl();
}

// Reconstructed from eboot.elf at 0x8E3350.
bool PS4ComputeBuffer::_SyncStaticImpl() {
    _FreeImpl();
    _AllocateStorage();
    return true;
}

// Reconstructed from eboot.elf at 0x8E34D0.
void PS4ComputeBuffer::_FreeImpl() {
    for (auto& storage : mStorage) {
        PS4DeferredDelete(storage);
        storage = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x8E3510.
void PS4ComputeBuffer::_SyncDynamicImpl(RndContext&) {
    mActiveBank = (mActiveBank & 1U) == 0 ? 1 : 0;
    std::memcpy(mStorage[mActiveBank], mStagingData, mStagingSize);
}

// Reconstructed from eboot.elf at 0x8E3540.
void PS4ComputeBuffer::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E37D0.
void PS4ComputeBuffer::_GpuCopyFromImpl(RndContext&, RndShaderResource&) {}

// Reconstructed from eboot.elf at 0x8E3580.
void PS4ComputeBuffer::_SelectForVSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int) {
    _Select(context, kShaderProgramVertex, slot, 0);
}

// Reconstructed from eboot.elf at 0x8E3600.
void PS4ComputeBuffer::_SelectForHSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int) {
    _Select(context, kShaderProgramHull, slot, 0);
}

// Reconstructed from eboot.elf at 0x8E3630.
void PS4ComputeBuffer::_SelectForDSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int) {
    _Select(context, kShaderProgramDomain, slot, 0);
}

// Reconstructed from eboot.elf at 0x8E3660.
void PS4ComputeBuffer::_SelectForGSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int) {
    _Select(context, kShaderProgramGeometry, slot, 0);
}

// Reconstructed from eboot.elf at 0x8E3690.
void PS4ComputeBuffer::_SelectForPSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int flags) {
    _Select(context, kShaderProgramPixel, slot, flags);
}

// Reconstructed from eboot.elf at 0x8E36D0.
void PS4ComputeBuffer::_SelectForCSImpl(
    RndContext& context,
    unsigned long slot,
    unsigned int flags,
    unsigned long) {
    _Select(context, kShaderProgramCompute, slot, flags);
}
