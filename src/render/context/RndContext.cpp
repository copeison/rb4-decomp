#include "render/context/RndContext.h"

#include <cstring>

#include "math/vector/Vector4.h"
#include "os/platform/PlatformMgr.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"

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
      mTargetMode(kTargetModeNone),
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
void RndContext::_BeginFrameImpl(unsigned int) {}
void RndContext::_SetActivePipelineImpl(int, unsigned long) {}
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

    const bool clipped = mClipPlanes[0].mEnabled || mClipPlanes[1].mEnabled ||
        mClipPlanes[2].mEnabled || mClipPlanes[3].mEnabled;
    auto* clipPlanes = clipped ? mCBuffers[2] : device->mBuiltinCBuffers[1];
    clipPlanes->_SelectImpl(*this);
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

// Reconstructed from eboot.elf at 0x6BD8A0.
void RndContext::SetActivePipeline(RndPipeline pipeline, unsigned long computeSlot) {
    const auto previousPipeline = mActivePipe;
    unsigned long slot = 0;
    if (TheRndDevice()->mSettings->mAsyncComputeEnabled) {
        slot = computeSlot;
    } else {
        pipeline = kPipelineGraphics;
    }
    const auto previousSlot = mActiveComputeSlot;
    if (pipeline != previousPipeline || slot != previousSlot) {
        mActivePipe = pipeline;
        mActiveComputeSlot = slot;
        _SetActivePipelineImpl(previousPipeline, previousSlot);
    }
}

// Reconstructed from eboot.elf at 0x6BC3B0.
void RndContext::BeginFrame(unsigned int flags) {
    mTargetMode = kTargetModeNone;
    mUnknown24.mSize = 0;
    mCameraCBufferOverride = nullptr;
    std::memset(mUnknown120, 0, sizeof(mUnknown120));
    mRenderTargetWidth = 0.0F;
    mRenderTargetHeight = 0.0F;
    std::memset(mUnknown136, 0, sizeof(mUnknown136));
    mUnknown140 = 1.0F;
    mCameras[0].Clear();
    mCameras[1].Clear();
    mUsingIdentityViewProjection = false;
    mUnknown18776 = 0;
    RndCameraContext::SetDefaultShaderConstants(kTargetMode2D, *mCBuffers[1]);
    mBlendMode = RndBlendMode::kSource;
    mShadingMode = kShadingModeStandard;
    mUnknown18972 = -1;
    mUnknown18976 = -1;
    mActiveShaderStages = 0;
    std::memset(mUnknown19096, 0xFF, sizeof(mUnknown19096));
    for (auto& limit : mInputSlotLimits) {
        limit = 0;
    }
    for (auto& limit : mOutputSlotLimits) {
        limit = 0;
    }
    _BeginFrameImpl(flags);

    auto* device = TheRndDevice();
    device->mBuiltinCBuffers[0]->_SelectImpl(*this);
    for (auto& plane : mClipPlanes) {
        plane.mEnabled = false;
    }
    _SyncClipPlanes(0xFFFFFFFF);

    auto& drawState = *mCBuffers[3];
    auto& shaders = device->mShaderMgr;
    *static_cast<float*>(RndShaderDrawUtl::GetCBufferMember(drawState, shaders.mEnvironIndex)) = -1.0F;
    drawState.mSyncPending = true;
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(drawState, shaders.mSolidColor),
        &Hmx::Color::GetZero(),
        sizeof(Hmx::Color));
    drawState.mSyncPending = true;
    device->mBuiltinCBuffers[2]->_SelectImpl(*this);
    device->mBuiltinCBuffers[3]->_SelectImpl(*this);
}

// Reconstructed from eboot.elf at 0x6BC590. Only platforms that support
// clip planes write them.
void RndContext::_SyncClipPlanes(unsigned int mask) {
    auto* device = TheRndDevice();
    if (!device->mCapabilities[kPlatformPS4].mEnabled) {
        return;
    }
    for (unsigned int index = 0; index < 4; ++index) {
        if ((mask & (1U << index)) == 0) {
            continue;
        }
        auto& cbuffer = *mCBuffers[2];
        const auto& plane = mClipPlanes[index];
        std::memcpy(
            RndShaderDrawUtl::GetCBufferMember(cbuffer, device->mShaderMgr.mClipPlanes + index),
            plane.mEnabled ? static_cast<const void*>(plane.mPlane) : &Vector4::sZero,
            sizeof(Vector4));
        cbuffer.mSyncPending = true;
    }

    const bool clipped = mClipPlanes[0].mEnabled || mClipPlanes[1].mEnabled ||
        mClipPlanes[2].mEnabled || mClipPlanes[3].mEnabled;
    if (clipped) {
        auto* cbuffer = mCBuffers[2];
        if (cbuffer->mSyncPending) {
            cbuffer->_SyncImpl(*this, 0, cbuffer->mNumElements);
            cbuffer->mSyncPending = false;
        }
        cbuffer->_SelectImpl(*this);
    } else {
        device->mBuiltinCBuffers[1]->_SelectImpl(*this);
    }
}

// Reconstructed from eboot.elf at 0x6BDA00.
RndScopedGpuStatBlock::RndScopedGpuStatBlock(RndContext& context, const char* name)
    : mContext(context),
      mKey(TheRndDevice()->mGpuStats.BeginStatBlock(context, name)) {}

// Reconstructed from eboot.elf at 0x6BDA30.
RndScopedGpuStatBlock::~RndScopedGpuStatBlock() {
    TheRndDevice()->mGpuStats.EndStatBlock(mContext, mKey);
}
