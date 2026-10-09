#include "render/system/RndDevice.h"

#include <algorithm>
#include <array>

#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/system/RndWindow.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_frame_adapters.h"
#include "render/targets/RndBufferCollection.h"
#include "render/resources/audio/audio_analysis_textures.h"
#include "render/resources/lighting/render_lighting_resources_adapters.h"
#include "render/resources/meshes/primitive_mesh_set.h"
#include "render/lighting/fog/RndShaderFogDeferred.h"
#include "render/resources/video/bink_render_manager.h"
#include "render/resources/video/bink_render_manager_adapters.h"
#include "utl/time/Timer.h"

using namespace rb4;

RndDevice* gRndDevice = nullptr;

namespace {

constexpr std::size_t kPlatformConfigCount = 13;
constexpr std::size_t kCurrentPlatformConfig = 7;
constexpr std::size_t kFloatsPerCBufferElement = 4;
constexpr std::size_t kMaxSubmissionBarriers = 12;

float* CBufferElement(RndShaderCBuffer& buffer, unsigned long index) {
    return static_cast<float*>(buffer.mData) + kFloatsPerCBufferElement * index;
}

RndShaderCBuffer* NewBuiltinCBuffer(const RndShaderCBufferConfig& config) {
    return RndShaderCBuffer::New(config, 1);
}

void FinishBuiltinCBuffer(RndShaderCBuffer& buffer) {
    if (buffer.mSyncPending) {
        buffer._CreateImpl();
        buffer.mSyncPending = false;
    }
}

void UpdateFramePhase(const RndDevice& device) {
    render_frame_phase_callbacks(false);
    if (device.mSettings->partial_framerate_enabled) {
        render_set_partial_frame_phase(device.mFrameCount & 1U);
    }
}

// Reconstructed from the main-window branch of eboot.elf at 0x3DE4A0. Every
// window's targets leave the render-target state before submission.
void TransitionWindowTargets(RndDevice& device, RndContext& context) {
    std::array<RndResourceBarrier, kMaxSubmissionBarriers> barriers{};
    unsigned long count = 0;

    const auto& windows = device.mFrameWindows;
    for (unsigned long i = 0; i < windows.mSize; ++i) {
        const auto& window = *windows.mData[i];
        if (window.GetSize().empty()) {
            continue;
        }

        const auto targets = window.GetBufferCollections();
        for (std::size_t t = 0; t < targets.mCount; ++t) {
            auto& barrier = barriers[count++];
            barrier.mResource = targets.mCollections[t]->mBackBuffer;
            barrier.mSubresource = ~0UL;
            barrier.mBefore = RndResourceState::kRenderTarget;
        }
    }

    context._ResourceBarrierImpl(count, barriers.data());
}

}  // namespace

// Reconstructed from eboot.elf at 0x3DD410.
RndDevice::RndDevice()
    : mLockOwner(nullptr),
      mInitialized(false),
      mInitParams{true, true, true, 0},
      mImmediateContext(nullptr),
      mBeginFramePending(false),
      mBeginFrameFlags(0),
      mHdrOutputMode(0),
      mMainWindow(nullptr),
      mCurrentWindow(nullptr),
      mFrameCount(0),
      mOffscreenFrameCount(0),
      mInFrame(false),
      mTerminating(false),
      mFrameWindows{mFrameWindows.mStorage, 0, 6, {}},
      mLastFrameCycles(0),
      mPendingFrameCycles(0),
      mFrameTimerState(0),
      mGpuTotalStat(-1),
      mFrameRate(0.0F),
      mSmoothedFrameRate(0.0F),
      mSettings(nullptr),
      mFactory(nullptr),
      mFogDeferred(nullptr),
      mPrimitiveMeshes(nullptr),
      mAudioTextures(nullptr),
      mBuiltinCBuffers{},
      mUnknown3792(nullptr),
      mConsoleState() {
    render_lighting_resources_construct(mLighting);
    render_gpu_stat_block_construct(mGpuStats);
    gRndDevice = this;

    for (const auto platform : render_supported_platform_ids()) {
        if (platform < kPlatformConfigCount) {
            render_platform_config_initialize(
                mPlatformConfigs[platform], platform);
        }
    }

    // The original calls this predicate for platform seven and ignores the
    // result.
    render_platform_config_boot_probe(mPlatformConfigs[kCurrentPlatformConfig]);

    auto* settings = render_settings_allocate();
    render_settings_initialize(*settings);
    mSettings = settings;
}

// Reconstructed from eboot.elf at 0x3DD790. The members with destructors
// (the pending-free queue, the default resources, the platform
// configurations, the vectors, and the CritSecs) are destroyed after the body.
RndDevice::~RndDevice() {
    render_settings_release(mSettings);
    mSettings = nullptr;

    render_gpu_stat_block_destruct(mGpuStats);
    render_lighting_resources_destruct(mLighting);
}

void RndDevice::_ProcessDeferredDeletion() {}
void RndDevice::_AcquireDeferredContextImpl(RndContext*) {}
void RndDevice::_UnknownSlot9Impl() {}
void RndDevice::_ReleaseDeferredContextImpl(RndContext*) {}
void RndDevice::_ExecuteDeferredContextImpl(RndContext*) {}
void RndDevice::_SetConsoleStateImpl(ConsoleState) {}
int RndDevice::_GetGpuBlockingBehaviorImpl() const { return 0; }
int RndDevice::_UnknownSlot14Impl() { return 0; }
RndDeviceSlot15Result RndDevice::_UnknownSlot15Impl() { return {}; }
void RndDevice::_UnknownSlot16Impl() {}

// Reconstructed from eboot.elf at 0x3DDAE0.
void RndDevice::Init(const RndInitParams& params) {
    mInitialized = true;
    mInitParams = params;
    mShaderMgr.PreInit();
    _InitImpl(&params);
    mShaderMgr.Init();

    render_lighting_resources_initialize(mLighting);
    mFogDeferred = new RndShaderFogDeferred;  // 0x451C90
    mPrimitiveMeshes = static_cast<RenderPrimitiveMeshSet*>(
        operator new(sizeof(RenderPrimitiveMeshSet)));
    render_primitive_mesh_set_construct(*mPrimitiveMeshes);
    mAudioTextures = static_cast<AudioAnalysisTextureSet*>(
        operator new(sizeof(AudioAnalysisTextureSet)));
    audio_analysis_texture_set_construct(*mAudioTextures);
    render_gpu_stat_block_initialize(mGpuStats);
    _InitBuiltinCBuffers();

    mImmediateContext->Init();
    for (auto* context : mDeferredContexts) {
        context->Init();
    }

    _PostInitImpl();

    const auto timerState = mFrameTimerState;
    if ((timerState & 0x80000000U) == 0) {
        mFrameTimerState = timerState + 1;
        if (timerState == 0) {
            mLastFrameCycles = Hmx::Timer::GetCycleCounter();
        }
    }
}

// Reconstructed from eboot.elf at 0x3DDC20.
void RndDevice::_InitBuiltinCBuffers() {
    const auto& constants = mShaderMgr;

    mBuiltinCBuffers[0] = NewBuiltinCBuffer(*constants.mRenderTargetCBuffer);
    auto* dimensions = CBufferElement(
        *mBuiltinCBuffers[0], constants.mTargetDimensions);
    dimensions[0] = 0.0F;
    dimensions[1] = 0.0F;
    FinishBuiltinCBuffer(*mBuiltinCBuffers[0]);

    mBuiltinCBuffers[1] = NewBuiltinCBuffer(*constants.mClipPlanesCBuffer);
    std::fill_n(
        CBufferElement(*mBuiltinCBuffers[1], constants.mClipPlanes), 16, 0.0F);
    FinishBuiltinCBuffer(*mBuiltinCBuffers[1]);

    mBuiltinCBuffers[2] = NewBuiltinCBuffer(*constants.mMiscDrawStateCBuffer);
    CBufferElement(*mBuiltinCBuffers[2], constants.mEnvironIndex)[0] =
        -1.0F;
    std::fill_n(
        CBufferElement(*mBuiltinCBuffers[2], constants.mSolidColor),
        kFloatsPerCBufferElement,
        0.0F);
    FinishBuiltinCBuffer(*mBuiltinCBuffers[2]);

    mBuiltinCBuffers[3] = NewBuiltinCBuffer(*constants.mOcclusionQueryCBuffer);
    auto* coverage = CBufferElement(
        *mBuiltinCBuffers[3], constants.mOcclusionQueryCoverageParams);
    coverage[0] = 0.0F;
    coverage[1] = 1.0F;
    FinishBuiltinCBuffer(*mBuiltinCBuffers[3]);
}

// Reconstructed from eboot.elf at 0x3DDE60.
void RndDevice::Terminate() {
    mTerminating = true;
    _ProcessPendingFrees();
    render_release_default_resources(mDefaults);
    delete mFogDeferred;  // 0x451CC0
    mFogDeferred = nullptr;
    render_lighting_resources_shutdown(mLighting);
    mShaderMgr.Terminate();

    if (mPrimitiveMeshes != nullptr) {
        render_primitive_mesh_set_destruct(*mPrimitiveMeshes);
        operator delete(mPrimitiveMeshes);
        mPrimitiveMeshes = nullptr;
    }
    if (mAudioTextures != nullptr) {
        audio_analysis_texture_set_destruct(*mAudioTextures);
        operator delete(mAudioTextures);
        mAudioTextures = nullptr;
    }
    for (auto*& buffer : mBuiltinCBuffers) {
        RndShaderCBuffer::SafeDelete(buffer);
    }

    mImmediateContext->Terminate();
    for (auto* context : mDeferredContexts) {
        context->Terminate();
    }
    _TerminateImpl();
}

// Reconstructed from eboot.elf at 0x3DDFF0.
void RndDevice::_ProcessPendingFrees() {
    mPendingFreeCritSec.Enter();
    for (auto* data : mPendingFrees) {
        delete data;
    }
    mPendingFrees.clear();
    mPendingFreeCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x3DE080.
void RndDevice::Lock() {
    mCritSec.Enter();
    mLockOwner = scePthreadSelf();
}

// Reconstructed from eboot.elf at 0x3DE0B0.
void RndDevice::Unlock() {
    static_cast<void>(scePthreadSelf());
    mLockOwner = nullptr;
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x3DE0E0.
void RndDevice::PollMainWindow() {
    ScopedCritSec lock(mCritSec);
    if (mMainWindow != nullptr) {
        mMainWindow->Poll();
    }
}

// Reconstructed from eboot.elf at 0x3DE130.
bool RndDevice::BeginMainWindowFrame() {
    _DoBeginFrame(false);
    if (_DoBeginDrawingWindow(*mMainWindow)) {
        return true;
    }
    _DoEndFrame(false);
    return false;
}

// Reconstructed from eboot.elf at 0x3DE170.
void RndDevice::_DoBeginFrame(bool offscreen) {
    Lock();
    mInFrame = true;
    _BeginFrameImpl(offscreen);

    if (!offscreen) {
        if (_GetGpuBlockingBehaviorImpl() == 0) {
            UpdateFramePhase(*this);
        }

        const auto now = Hmx::Timer::GetCycleCounter();
        const auto elapsed = mFrameTimerState != 0
            ? now - mLastFrameCycles
            : mPendingFrameCycles;
        mLastFrameCycles = now;
        mPendingFrameCycles = 0;
        mFrameTimerState = 1;

        mFrameRate = static_cast<float>(1000.0 / Hmx::Timer::CyclesToMs(elapsed));
        if (mSmoothedFrameRate == 0.0F) {
            mSmoothedFrameRate = mFrameRate;
        }
        mSmoothedFrameRate = (mSmoothedFrameRate * 59.0F + mFrameRate) / 60.0F;
    }

    mImmediateContext->mFrameActive = true;
    if (mBeginFramePending) {
        _FlushPendingBeginFrame();
    }
    mGpuTotalStat =
        render_gpu_stat_block_begin(mGpuStats, *mImmediateContext, "GPU Total");
    audio_analysis_textures_prepare_frame(*mAudioTextures, *mImmediateContext);
    bink_render_manager_prepare_frame(
        bink_render_manager_instance(), *mImmediateContext);
}

// Reconstructed from eboot.elf at 0x3DE3A0.
bool RndDevice::_DoBeginDrawingWindow(RndWindow& window) {
    static_cast<void>(scePthreadSelf());
    mCurrentWindow = &window;
    window.CheckForResize();

    if (window.GetSize().empty()) {
        static_cast<void>(scePthreadSelf());
        mCurrentTargets.clear();
        mCurrentWindow = nullptr;
        return false;
    }

    mFrameWindows.mData[mFrameWindows.mSize++] = &window;

    const auto targets = window.GetBufferCollections();
    mCurrentTargets.resize(targets.mCount);
    std::copy_n(targets.mCollections, targets.mCount, mCurrentTargets.begin());

    mImmediateContext->BeginFrame(0);
    return true;
}

// Reconstructed from eboot.elf at 0x3DE4A0.
void RndDevice::_DoEndFrame(bool offscreen) {
    if (mBeginFramePending) {
        _FlushPendingBeginFrame();
    }
    render_gpu_stat_block_end(mGpuStats, *mImmediateContext, mGpuTotalStat);

    if (offscreen) {
        _EndFrameImpl(mFrameWindows, true);
        ++mOffscreenFrameCount;
    } else {
        render_gpu_stat_block_finish_frame(mGpuStats);
        TransitionWindowTargets(*this, *mImmediateContext);
        _EndFrameImpl(mFrameWindows, false);
        if (_GetGpuBlockingBehaviorImpl() == 1) {
            UpdateFramePhase(*this);
        }
        mFrameWindows.mSize = 0;
        ++mFrameCount;
        _ProcessPendingFrees();
    }

    mInFrame = false;
    mImmediateContext->mFrameActive = false;
    Unlock();

    if (!offscreen) {
        render_poll_default_resources(mDefaults);
    }
}

// Reconstructed from eboot.elf at 0x3DE7C0.
void RndDevice::EndMainWindowFrame() {
    static_cast<void>(scePthreadSelf());
    mCurrentTargets.clear();
    mCurrentWindow = nullptr;
    _DoEndFrame(false);
}

// Reconstructed from eboot.elf at 0x3DE8E0.
void RndDevice::_DoEndDrawingBufferCollection() {
    mCurrentTargets.clear();
}

// Reconstructed from eboot.elf at 0x3DE8F0.
void RndDevice::BeginOffscreenFrame(RndBufferCollection* buffers) {
    _DoBeginFrame(true);
    mCurrentTargets.resize(buffers == nullptr ? 0 : 1);
    if (buffers != nullptr) {
        mCurrentTargets[0] = buffers;
    }
    mImmediateContext->BeginFrame(0);
}

// Reconstructed from eboot.elf at 0x3DE9E0.
void RndDevice::EndOffscreenFrame() {
    _DoEndDrawingBufferCollection();
    _DoEndFrame(true);
}

// Reconstructed from eboot.elf at 0x3DEAA0.
void RndDevice::ForceIncrementFrameCount() {
    ScopedCritSec lock(mCritSec);
    ++mFrameCount;
}

// Reconstructed from eboot.elf at 0x3DEB20.
RndContext* RndDevice::AcquireDeferredContext(unsigned long index) {
    auto* context = mDeferredContexts[index];
    context->mMode = 1;
    context->mFrameActive = true;
    _AcquireDeferredContextImpl(context);
    context->BeginFrame(1);
    return context;
}

// Reconstructed from eboot.elf at 0x3DEB60.
void RndDevice::ReleaseDeferredContext(unsigned long index) {
    auto* context = mDeferredContexts[index];
    context->mMode = 2;
    context->mFrameActive = false;
    _ReleaseDeferredContextImpl(context);
}

// Reconstructed from eboot.elf at 0x3DEB80.
void RndDevice::ExecuteDeferredContext(unsigned long index) {
    auto* context = mDeferredContexts[index];
    context->mMode = 0;
    mBeginFramePending = false;
    _ExecuteDeferredContextImpl(context);
    mBeginFramePending = true;
    mBeginFrameFlags |= 1;
}

// Reconstructed from eboot.elf at 0x3DEC00.
RndDevice::ConsoleState RndDevice::GetConsoleState() const {
    return mConsoleState;
}

// Reconstructed from eboot.elf at 0x3DEC10.
void RndDevice::SetConsoleState(ConsoleState state) {
    _SetConsoleStateImpl(state);
}

// Reconstructed from eboot.elf at 0x3DEC20. Data freed during termination
// is deleted at once; otherwise it waits for the end of the frame.
void RndDevice::SyncFreeMaterialData(RndMaterialRuntimeData* data) {
    if (mTerminating) {
        delete data;
        return;
    }
    mPendingFreeCritSec.Enter();
    mPendingFrees.emplace_back(data);
    mPendingFreeCritSec.Exit();
}

// Reconstructed from eboot.elf at 0x3DED70.
void RndDevice::_InstallMainWindow(RndWindow* window) {
    mMainWindow = window;
}

// Reconstructed from eboot.elf at 0x3DED80.
void RndDevice::_DestroyMainWindow() {
    if (mMainWindow != nullptr) {
        delete mMainWindow;
        mMainWindow = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x3DEDB0.
void RndDevice::_InstallFactory(RndFactory* factory) {
    mFactory = factory;
}

// Reconstructed from eboot.elf at 0x3DEDC0.
void RndDevice::_InstallImmediateContext(RndContext* context) {
    mImmediateContext = context;
}

// Reconstructed from eboot.elf at 0x3DEEA0.
void RndDevice::_DestroyContexts() {
    if (mImmediateContext != nullptr) {
        delete mImmediateContext;
        mImmediateContext = nullptr;
    }
    while (!mDeferredContexts.empty()) {
        auto* context = mDeferredContexts.back();
        mDeferredContexts.pop_back();
        delete context;
    }
}

// Reconstructed from eboot.elf at 0x3DEF20.
void RndDevice::_FlushPendingBeginFrame() {
    mImmediateContext->BeginFrame(mBeginFrameFlags);
    mBeginFramePending = false;
    mBeginFrameFlags = 0;
}
