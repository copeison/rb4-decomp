#include "render/context/RndContext.h"

#include <cstring>

#include "render/buffers/RndShaderCBuffer.h"
#include "render/system/RndDevice.h"
#include "render/resources/system/render_resource_manager.h"

namespace {

constexpr unsigned long kReservedRecords = 2000;

}  // namespace

// Reconstructed from eboot.elf at 0x6BBDE0. Contexts that disable compute
// queues start in mode 0, others in mode -1. The 3168-byte table is filled
// with 0xFF, and 2000 sixteen-byte records are reserved up front.
RndContext::RndContext(bool disableComputeQueues)
    : mFrameActive(false),
      mDisableComputeQueues(disableComputeQueues),
      mMode(disableComputeQueues ? 0 : -1),
      mSliceMode(-1),
      mUnknown112{},
      mUnknown140(1.0F),
      mUnknown18768(false),
      mUnknown18776(0),
      mActiveShaderStages(0),
      mUnknown18788(5),
      mInputSlotLimits{},
      mOutputSlotLimits{},
      mUnknown18888(false),
      mUnknown18892(0),
      mUnknown18896(0),
      mUnknown18900(1.0F),
      mUnknown18904(0),
      mUnknown18908{},
      mShadingMode(0),
      mUnknown18972(-1),
      mUnknown18976(-1),
      mUnknown18980{},
      mCBuffers{},
      mUnknown22304(false) {
    mUnknown24.mData = mUnknown24.mStorage;
    mUnknown24.mSize = 0;
    mUnknown24.mCapacity = 8;
    // The constructor also copies two 16-byte constants into +0x49E0..+0x4A18;
    // their values have not been recovered.
    std::memset(mUnknown19096, 0xFF, sizeof(mUnknown19096));
    if (mUnknown18992.capacity() < kReservedRecords) {
        mUnknown18992.reserve(kReservedRecords);
    }
}

// Reconstructed from eboot.elf at 0x6BC120. The deleting destructor is at
// 0x6BC190.
RndContext::~RndContext() {}

// Reconstructed from eboot.elf at 0x6BC200. Creates the context's global
// constant buffers from the shader manager's configurations.
void RndContext::Init() {
    const auto& constants =
        TheRndDevice()->mResourceMgr
            .shader_constants;
    mCBuffers[0] = RndShaderCBuffer::New(*constants.render_target_block, 0);
    mCBuffers[1] = RndShaderCBuffer::New(*constants.camera_block, 0);
    mCBuffers[2] = RndShaderCBuffer::New(*constants.clip_planes_block, 0);
    mCBuffers[3] = RndShaderCBuffer::New(*constants.misc_draw_state_block, 0);
    mCBuffers[4] = RndShaderCBuffer::New(*constants.occlusion_query_block, 0);
    mCBuffers[5] = RndShaderCBuffer::New(*constants.debug_block, 0);
    mCBuffers[6] = RndShaderCBuffer::New(*constants.transient_blocks[0], 0);
    mCBuffers[7] = RndShaderCBuffer::New(*constants.transient_blocks[1], 0);
    mCBuffers[8] = RndShaderCBuffer::New(*constants.transient_blocks[2], 0);
}

// Reconstructed from eboot.elf at 0x6BC330.
void RndContext::Terminate() {
    for (auto*& buffer : mCBuffers) {
        RndShaderCBuffer::SafeDelete(buffer);
    }
}

void RndContext::_SignalFenceImpl(RndFence&) {}
void RndContext::_WaitFenceImpl(const RndFence&) {}
void RndContext::_FinishImpl() {}
void RndContext::_BeginFrameImpl() {}
void RndContext::_SetActivePipelineImpl(int) {}
void RndContext::_ResourceBarrierImpl(unsigned long, const RndResourceBarrier*) {}
void RndContext::_PushMarkerImpl(const char*) {}
void RndContext::_PopMarkerImpl() {}
void RndContext::_BeginGpuStatsImpl(unsigned long) {}
void RndContext::_EndGpuStatsImpl(unsigned long) {}

RndGpuStatSample RndContext::_EvalAndRetireGpuStatsImpl(unsigned long) {
    return {};
}

void RndContext::DeactivateShaderProgramType(RndShaderProgramType type) {
    mActiveShaderStages = static_cast<unsigned char>(mActiveShaderStages & ~(1U << type));
    _DeactivateShaderProgramTypeImpl(type);
}

RndGpuStatScope* RndContext::LastGpuStatScope() {
    return mGpuStatScopes.empty() ? nullptr : mGpuStatScopes.end() - 1;
}

void RndContext::PushGpuStatScope(const RndGpuStatScope& scope) {
    mGpuStatScopes.emplace_back(scope);
}

void RndContext::PopGpuStatScope() {
    --mGpuStatScopes.mpEnd;
}
