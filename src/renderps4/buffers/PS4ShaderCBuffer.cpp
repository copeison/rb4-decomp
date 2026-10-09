#include "renderps4/buffers/PS4ShaderCBuffer.h"

#include <algorithm>
#include <cstring>
#include <gnm/buffer.h>

#include "render/system/RndDevice.h"
#include "renderps4/context/PS4Context.h"
#include "renderps4/system/PS4Device.h"

namespace {

constexpr unsigned long kElementSize = 16;

// RndShaderCBuffer::mStageMask bits. Names not in the reference map.
constexpr unsigned int kStageVertex = 1;
constexpr unsigned int kStageTessellation = 2;
constexpr unsigned int kStageGeometry = 4;
constexpr unsigned int kStagePixel = 8;
constexpr unsigned int kStageCompute = 0x10;

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

// Reconstructed from eboot.elf at 0x8E39C0. The constants are copied into
// the active command buffer once per render frame and bound to every stage
// in the buffer's stage mask.
void PS4ShaderCBuffer::_SelectImpl(RndContext& context) {
    auto& ps4 = static_cast<PS4Context&>(context);
    const auto slot = static_cast<int>(mIndex);
    const auto stages = mStageMask;

    const auto frame = TheRndDevice()->mFrameCount;
    if (mFrame != frame) {
        mFrame = frame;
        mFrameData = nullptr;
    }
    if (mFrameData == nullptr) {
        const auto size = static_cast<unsigned int>(mGpuSize);
        if (context.mActivePipe == 1) {
            mFrameData = ps4._ActiveComputeContext().allocateFromCommandBuffer(
                size, sce::Gnm::kEmbeddedDataAlignment4);
        } else if (context.mActivePipe == 0) {
            mFrameData = ps4._ActiveGfxContext().allocateFromCommandBuffer(
                size, sce::Gnm::kEmbeddedDataAlignment4);
        }
    }
    std::memcpy(mFrameData, mGpuData, mGpuSize);

    sce::Gnm::Buffer buffer;
    buffer.initAsConstantBuffer(mFrameData, static_cast<unsigned int>(mGpuSize));
    buffer.setResourceMemoryType(sce::Gnm::kResourceMemoryTypeRO);

    if (context.mActivePipe == 0) {
        auto& gfx = ps4._ActiveGfxContext();
        if ((stages & kStageVertex) != 0) {
            gfx.setConstantBuffers(sce::Gnm::kShaderStageVs, slot, 1, &buffer);
        }
        if ((stages & kStageTessellation) != 0) {
            gfx.setConstantBuffers(sce::Gnm::kShaderStageHs, slot, 1, &buffer);
            gfx.setConstantBuffers(sce::Gnm::kShaderStageLs, slot, 1, &buffer);
        }
        if ((stages & kStageGeometry) != 0) {
            gfx.setConstantBuffers(sce::Gnm::kShaderStageGs, slot, 1, &buffer);
        }
        if ((stages & kStagePixel) != 0) {
            gfx.setConstantBuffers(sce::Gnm::kShaderStagePs, slot, 1, &buffer);
        }
    }
    if ((stages & kStageCompute) != 0) {
        if (context.mActivePipe == 1) {
            ps4._ActiveComputeContext().setConstantBuffers(slot, 1, &buffer);
        } else if (context.mActivePipe == 0) {
            ps4._ActiveGfxContext().setConstantBuffers(
                sce::Gnm::kShaderStageCs, slot, 1, &buffer);
        }
    }
}
