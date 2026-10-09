#include "render/context/RndContext.h"

#include <cstring>

#include "render/buffers/RndShaderCBuffer.h"
#include "render/system/RndDevice.h"
#include "render/shaders/RndShaderMgr.h"

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
      mCameraCBufferOverride(nullptr),
      mUnknown120{},
      mRenderTargetWidth(0.0F),
      mRenderTargetHeight(0.0F),
      mUnknown136{},
      mUnknown140(1.0F),
      mUsingIdentityViewProjection(false),
      mUnknown18776(0),
      mActiveShaderStages(0),
      mBlendMode(RndBlendMode::kSource),
      mInputSlotLimits{},
      mOutputSlotLimits{},
      mShadingMode(kShadingModeStandard),
      mUnknown18972(-1),
      mUnknown18976(-1),
      mActivePipe(0),
      mActiveComputeSlot(0),
      mCBuffers{},
      mUnknown22304(false) {
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
    const auto& constants = TheRndDevice()->mShaderMgr;
    mCBuffers[0] = RndShaderCBuffer::New(*constants.mRenderTargetCBuffer, 0);
    mCBuffers[1] = RndShaderCBuffer::New(*constants.mCameraCBuffer, 0);
    mCBuffers[2] = RndShaderCBuffer::New(*constants.mClipPlanesCBuffer, 0);
    mCBuffers[3] = RndShaderCBuffer::New(*constants.mMiscDrawStateCBuffer, 0);
    mCBuffers[4] = RndShaderCBuffer::New(*constants.mOcclusionQueryCBuffer, 0);
    mCBuffers[5] = RndShaderCBuffer::New(*constants.mDebugCBuffer, 0);
    mCBuffers[6] = RndShaderCBuffer::New(*constants.mTransientCBuffers[0], 0);
    mCBuffers[7] = RndShaderCBuffer::New(*constants.mTransientCBuffers[1], 0);
    mCBuffers[8] = RndShaderCBuffer::New(*constants.mTransientCBuffers[2], 0);
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

// Reconstructed from eboot.elf at 0x6BD930.
void RndContext::_ReselectGlobalCBuffers() {
    auto* device = TheRndDevice();
    mCBuffers[1]->_SelectImpl(*this);
    mCBuffers[5]->_SelectImpl(*this);
    device->mBuiltinCBuffers[2]->_SelectImpl(*this);
    device->mBuiltinCBuffers[3]->_SelectImpl(*this);

    auto* camera = mUnknown24.mSize != 0 || mCameraCBufferOverride != nullptr
        ? mCBuffers[0]
        : device->mBuiltinCBuffers[0];
    camera->_SelectImpl(*this);

    const bool lit = mLightSlots[0].mEnabled || mLightSlots[1].mEnabled ||
        mLightSlots[2].mEnabled || mLightSlots[3].mEnabled;
    auto* lights = lit ? mCBuffers[2] : device->mBuiltinCBuffers[1];
    lights->_SelectImpl(*this);
}

// Reconstructed from eboot.elf at 0x6BD340. An overriding camera keeps its
// constants.
void RndContext::SetUsingIdentityViewProjection(bool identity) {
    if (mUsingIdentityViewProjection == identity) {
        return;
    }
    mUsingIdentityViewProjection = identity;
    if (mUnknown18776 == 0) {
        _SyncCameraCBuffer();
    }
}

// Reconstructed from eboot.elf at 0x6BD5D0.
void RndContext::SetShadingMode(RndShadingMode mode) {
    const auto previous = mShadingMode;
    if (previous == mode) {
        return;
    }
    mShadingMode = mode;
    if (mode == kShadingModeWireframe) {
        _SetFillModeImpl(false);
        _SetDepthBiasEnabledImpl(true);
    } else if (previous == kShadingModeWireframe) {
        _SetFillModeImpl(true);
        _SetDepthBiasEnabledImpl(false);
    }
}
