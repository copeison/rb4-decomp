#include "renderps4/buffers/PS4ShaderCBuffer.h"
#include "renderps4/system/PS4Device.h"

#include <algorithm>
#include <cstring>

namespace {

constexpr unsigned long kElementSize = 16;

}  // namespace

// Reconstructed from eboot.elf at 0x8E3800.
PS4ShaderCBuffer::PS4ShaderCBuffer(
    const RndShaderCBufferConfig& config,
    unsigned int flags,
    unsigned long numElements,
    void* data)
    : RndShaderCBuffer(config, flags, numElements, data),
      mFrameData(nullptr),
      mGpuData(nullptr),
      mGpuSize(0),
      mFrame(0) {}

// Reconstructed from eboot.elf at 0x8E3830. The deleting destructor at
// 0x8E38C0 releases the buffer through MemFree.
PS4ShaderCBuffer::~PS4ShaderCBuffer() {
    _FreeGpuData();
}

void PS4ShaderCBuffer::_FreeGpuData() {
    if (mGpuData != nullptr) {
        gPS4Device->DeferredDelete(mGpuData);
        mGpuData = nullptr;
    }
    mGpuSize = 0;
}

// Reconstructed from eboot.elf at 0x8E3900. The GPU copy is allocated with
// the label "CBuffer" and alignment argument 4.
void PS4ShaderCBuffer::_CreateImpl() {
    _FreeGpuData();
    mGpuSize = std::max(kElementSize, kElementSize * mNumElements);
    mGpuData = MemAlloc(mGpuSize, "CBuffer", 4);
    std::memcpy(mGpuData, mData, mGpuSize);
    mFrameData = nullptr;
}

// Reconstructed from eboot.elf at 0x8E3980.
void PS4ShaderCBuffer::_SyncImpl(
    RndContext&,
    unsigned long first,
    unsigned long end) {
    auto* destination =
        static_cast<unsigned char*>(mGpuData) + kElementSize * first;
    std::memcpy(destination, mData, kElementSize * (end - first));
    mFrameData = nullptr;
}

// Reconstructed from eboot.elf at 0x8E39C0.
void PS4ShaderCBuffer::_SelectImpl(RndContext& context) {
    _PrepareFrameData(context);
    _SelectStages(context);
}
