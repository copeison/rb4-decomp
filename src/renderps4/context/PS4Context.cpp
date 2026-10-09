#include "renderps4/context/PS4Context.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <gnm/platform.h>
#include <gnmx/surfacetool.h>

#include "os/memory/MemMgr.h"
#include "render/context/RndResourceBarrier.h"
#include "render/materials/RndMaterialCom.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndVertexInterpreter.h"
#include "render/shaders/RndShaderEnums.h"
#include "render/system/RndDevice.h"
#include "render/buffers/RndCShaderClearBuffer.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/shaders/RndShaderBasic.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/textures/RndTextureBase.h"
#include "renderps4/buffers/PS4ComputeBuffer.h"
#include "renderps4/textures/PS4Texture2D.h"
#include "renderps4/textures/PS4TextureArray2D.h"
#include "renderps4/textures/PS4TextureCube.h"
#include "renderps4/video/PS4Window.h"
#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4Fence.h"
#include "renderps4/system/PS4RenderUtl.h"

namespace {

constexpr std::size_t kTransientVertexCapacity = 0x40000;
constexpr std::size_t kCueSlotCount = 64;
constexpr std::uint32_t kDrawCommandBufferSize = 32 * 1024 * 1024;
constexpr std::uint32_t kConstantCommandBufferSize = 2 * 1024 * 1024;
constexpr std::size_t kGlobalResourceTableSize = 192;
constexpr std::uint32_t kGsRingSize = 4 * 1024 * 1024;
constexpr std::uint32_t kMaxGsVertexSizeInDwords = 100;
constexpr std::uint32_t kMaxGsOutputVertexCount = 0x100000;
// CUE slots per graphics context: resources, read-write resources,
// samplers and vertex buffers.
constexpr sce::Gnmx::ConstantUpdateEngine::RingSetup kCueRingSetup = {128, 16, 16, 32};
constexpr std::uint32_t kComputeCommandBufferSize = 0x3FFFFC;
constexpr std::size_t kComputeQueueRingSize = 4096;
constexpr std::size_t kComputeQueueRingAlignment = 256;

// Maps a compute queue with a 4 KiB ring, as SDK 2.500's inline
// ComputeQueue::map did with a priority. Name not in the reference map.
void MapComputeQueue(
    sce::Gnmx::ComputeQueue& queue,
    std::uint32_t pipe,
    sce::Gnm::PipePriority priority) {
    void* ring = MemAlloc(kComputeQueueRingSize, "ComputeQueue", kComputeQueueRingAlignment);
    std::memset(ring, 0, kComputeQueueRingSize);
    auto* readPtr = static_cast<std::uint32_t*>(MemAlloc(sizeof(std::uint32_t), "ComputeQueue", 16));
    queue.initialize(pipe, 0);
    sce::Gnm::mapComputeQueueWithPriority(
        &queue.m_vqueueId,
        queue.m_pipeId,
        queue.m_queueId,
        ring,
        kComputeQueueRingSize / sizeof(std::uint32_t),
        readPtr,
        priority);
    queue.m_dcbRoot.init(ring, kComputeQueueRingSize, nullptr, nullptr);
    queue.m_readPtrAddr = readPtr;
}
constexpr unsigned int kNumShaderStages = 6;
constexpr unsigned int kAllShaderStages = (1U << kNumShaderStages) - 1;
// The Gnm stage of each engine stage (vertex, hull, domain, geometry,
// pixel, compute), as the table at 0x12D19D0. Name not in the reference map.
constexpr sce::Gnm::ShaderStage kGnmShaderStages[kNumShaderStages] = {
    sce::Gnm::kShaderStageVs,
    sce::Gnm::kShaderStageHs,
    sce::Gnm::kShaderStageLs,
    sce::Gnm::kShaderStageGs,
    sce::Gnm::kShaderStagePs,
    sce::Gnm::kShaderStageCs,
};
constexpr unsigned int kNumRwTextureSlots = 128;
constexpr unsigned int kNumSourceSlots = 16;
constexpr std::size_t kTimestampBufferSize = 0x2000;
constexpr int kTimestampAlignment = 8;
constexpr std::size_t kInitialLabelCapacity = 32;
constexpr std::size_t kHighPriorityComputeContextCount = 3;
constexpr std::size_t kSubmissionCounterCount = 10;

// RndContext::mActiveShaderStages bits SetupDraw reads. Names not in the
// reference map.
constexpr unsigned int kTessellationStageBits =
    (1U << kShaderProgramHull) | (1U << kShaderProgramDomain);
constexpr unsigned int kGeometryStageBit = 1U << kShaderProgramGeometry;

constexpr std::size_t kColorRenderTargetCount = 8;
constexpr std::array<std::uint8_t, 10> kStencilMasks = {
    0xFF, 0x07, 0x08, 0x10, 0x0F,
    0x1F, 0x20, 0x28, 0x30, 0xC0,
};

constexpr std::uint32_t kDebugMarkerColor = 0xFF0000FF;
constexpr float kGpuClockSeconds = 1.25e-9F;

constexpr std::uint32_t kResourceReadyValue = 1;

std::uint8_t StencilMask(std::uint32_t index) {
    return index < kStencilMasks.size() ? kStencilMasks[index] : 0;
}

}  // namespace

// Construction and submission ------------------------------------------------

PS4Context* PS4Context::_CreateImmediate(PS4Device& device) {
    auto* context = new PS4Context;
    device._InstallImmediateContext(context);
    return context;
}

// Reconstructed from eboot.elf at 0x8E72B0.
PS4Context::PS4Context()
    : RndContext(false),
      mLabels(nullptr),
      mLabelFrameStarts{0, 0},
      mLabelFrame(0),
      mNextLabel(0),
      mLabelCapacity(0),
      mActiveFrame(0),
      mDepthMode(0),
      mStencilMode(0),
      mFrontFace(1),
      mCullMode(kCullNone),
      mFillMode(1),
      mStencilReference(0),
      mStencilReadMask(0xFF),
      mStencilWriteMask(0xFF),
      mColorWriteTargets(0xF),
      mColorWriteChannels(kWriteRGBA),
      mGpuTimestamps(nullptr),
      mNextGpuStatBlock(0),
      mCachedShaderStages(static_cast<sce::Gnm::ActiveShaderStages>(-1)),
      mCachedPrimitiveType(static_cast<sce::Gnm::PrimitiveType>(-1)),
      mGsModeEnabled(false),
      mCbEnabled(true) {
    _CreateGfxContext();
    _CreateGpuTimestampPool();

    for (std::size_t bank = 0; bank < kFrameSlotCount; ++bank) {
        for (std::size_t format = 0; format < kTransientFormatCount; ++format) {
            mTransientBuffers[bank][format].Init(
                static_cast<RndVertexType>(format), kTransientVertexCapacity);
        }
    }

    if (mDisableComputeQueues) {
        return;
    }

    MapComputeQueue(mComputeQueues[0], 1, sce::Gnm::kPipePriorityMedium);
    MapComputeQueue(mComputeQueues[1], 0, sce::Gnm::kPipePriorityLow);
    for (auto& frame : mComputeContexts) {
        for (auto& compute : frame) {
            const auto cueHeapSize = sce::Gnmx::ConstantUpdateEngine::computeHeapSize(kCueSlotCount);
            static long sGpuHeap = MemFindHeap("gpu");
            MemPushHeap(sGpuHeap);
            void* resourceBuffer = MemAlloc(cueHeapSize, "GfxContext", 4);
            MemPopHeap();
            void* commandBuffer = MemAlloc(kComputeCommandBufferSize, "ComputeContext", 4);
            compute.init(
                commandBuffer, kComputeCommandBufferSize, resourceBuffer, cueHeapSize, nullptr);
        }
    }
    mResourceSignals.reserve(kInitialLabelCapacity);
    mLabelCapacity = kInitialLabelCapacity;
    _AllocateResourceLabel();
}

// Reconstructed from eboot.elf at 0x8E7AF0. The global resource table and
// the GS rings are reallocated for each slot; only the last allocations are
// kept.
void PS4Context::_CreateGfxContext() {
    for (std::size_t slot = 0; slot < kFrameSlotCount; ++slot) {
        {
            static long sGpuHeap = MemFindHeap("gpu");
            MemPushHeap(sGpuHeap);
            mCueHeaps[slot] = MemAlloc(
                sce::Gnmx::ConstantUpdateEngine::computeHeapSize(kCueSlotCount), "GfxContext", 4);
            MemPopHeap();
        }
        mDrawCommandBuffers[slot] = MemAlloc(kDrawCommandBufferSize, "GfxContext", 4);
        mConstantCommandBuffers[slot] = MemAlloc(kConstantCommandBufferSize, "GfxContext", 4);
        for (auto& pending : mSubmissionPending[slot]) {
            pending = 0;
        }

        auto& gfx = mGfxContexts[slot];
        gfx.init(
            mCueHeaps[slot],
            kCueSlotCount,
            kCueRingSetup,
            mDrawCommandBuffers[slot],
            kDrawCommandBufferSize,
            mConstantCommandBuffers[slot],
            kConstantCommandBufferSize);
        {
            static long sGpuHeap = MemFindHeap("gpu");
            MemPushHeap(sGpuHeap);
            mGlobalResourceTable = MemAlloc(kGlobalResourceTableSize, "GfxContext", 4);
            MemPopHeap();
        }
        gfx.setGlobalResourceTableAddr(mGlobalResourceTable);
        {
            static long sGpuHeap = MemFindHeap("gpu");
            MemPushHeap(sGpuHeap);
            mEsGsRing = MemAlloc(kGsRingSize, "GfxContext", 4);
            mGsVsRing = MemAlloc(kGsRingSize, "GfxContext", 4);
            MemPopHeap();
        }
        gfx.setEsGsRingBuffer(mEsGsRing, kGsRingSize, kMaxGsVertexSizeInDwords);
        const std::uint32_t vertexSizes[4] = {
            kMaxGsVertexSizeInDwords,
            kMaxGsVertexSizeInDwords,
            kMaxGsVertexSizeInDwords,
            kMaxGsVertexSizeInDwords,
        };
        gfx.setGsVsRingBuffers(mGsVsRing, kGsRingSize, vertexSizes, kMaxGsOutputVertexCount);
    }
}

// Reconstructed from eboot.elf at 0x8E7DF0. Each block owns a begin and an
// end timestamp in one "gpu"-heap allocation.
void PS4Context::_CreateGpuTimestampPool() {
    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    mGpuTimestamps = static_cast<std::uint64_t*>(
        MemAlloc(kTimestampBufferSize, "GpuStatBlock timestamps", kTimestampAlignment));
    MemPopHeap();
    for (std::size_t index = 0; index < kNumGpuStatBlocks; ++index) {
        auto& block = mGpuStatBlocks[index];
        block.mActive = false;
        block.mBegin = mGpuTimestamps + 2 * index;
        block.mEnd = mGpuTimestamps + 2 * index + 1;
    }
}

// Reconstructed from eboot.elf at 0x8E8070.
PS4Context::~PS4Context() {
    MemFree(mGpuTimestamps);
    MemFree(mLabels);
}

bool PS4Context::_RecordingGraphics() const {
    return mActivePipe == 0;
}

bool PS4Context::_RecordingCompute() const {
    return mActivePipe == 1;
}

sce::Gnmx::ComputeContext& PS4Context::_ActiveComputeContext() {
    return mComputeContexts[mActiveFrame][mActiveComputeSlot];
}

sce::Gnmx::GfxContext& PS4Context::_ActiveGfxContext() {
    return mGfxContexts[mActiveFrame];
}

bool PS4Context::_SubmissionsComplete() const {
    return _FrameSubmissionsComplete(mActiveFrame);
}

bool PS4Context::_FrameSubmissionsComplete(std::size_t frame) const {
    for (std::size_t index = 0; index < kSubmissionCounterCount; ++index) {
        if (mSubmissionPending[frame][index] != 0) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x8E82D0. Each submission clears its
// pending flag when the GPU finishes it, the graphics one with an interrupt;
// the first three compute contexts go to the medium-priority queue.
void PS4Context::SubmitFrame() {
    _ActiveGfxContext().triggerEvent(sce::Gnm::kEventTypeCacheFlushAndInvEvent);
    for (std::size_t slot = 0; slot < kComputeContextsPerFrame; ++slot) {
        auto& compute = mComputeContexts[mActiveFrame][slot];
        auto& pending = mSubmissionPending[mActiveFrame][slot + 1];
        pending = 1;
        compute.m_dcb.writeReleaseMemEvent(
            sce::Gnm::kReleaseMemEventCsDone,
            sce::Gnm::kEventWriteDestMemory,
            const_cast<std::int32_t*>(&pending),
            sce::Gnm::kEventWriteSource32BitsImmediate,
            0,
            sce::Gnm::kCacheActionNone,
            sce::Gnm::kCachePolicyLru);
        mComputeQueues[slot < kHighPriorityComputeContextCount ? 0 : 1].submit(&compute);
    }

    auto& pending = mSubmissionPending[mActiveFrame][0];
    pending = 1;
    auto& gfx = _ActiveGfxContext();
    gfx.writeAtEndOfPipeWithInterrupt(
        sce::Gnm::kEopFlushCbDbCaches,
        sce::Gnm::kEventWriteDestMemory,
        const_cast<std::int32_t*>(&pending),
        sce::Gnm::kEventWriteSource32BitsImmediate,
        0,
        sce::Gnm::kCacheActionNone,
        sce::Gnm::kCachePolicyLru);
    gfx.submit();
    mActiveFrame = (mActiveFrame & 1) == 0 ? 1 : 0;
}

// Reconstructed from eboot.elf at 0x8E8450.
void PS4Context::_ResetFrame() {
    auto& gfx = _ActiveGfxContext();
    gfx.reset();
    gfx.initializeDefaultHardwareState();
    if (!mDisableComputeQueues) {
        for (auto& compute : mComputeContexts[mActiveFrame]) {
            compute.reset();
            compute.m_dcb.initializeDefaultHardwareState();
        }
    }

    _ResetDrawState();
    for (auto& buffer : mTransientBuffers[mActiveFrame]) {
        buffer.Reset();
    }
    sce::Gnm::ClipControl clip;
    clip.init();
    clip.setClipSpace(sce::Gnm::kClipControlClipSpaceDX);
    _ActiveGfxContext().setClipControl(clip);
}

// Inlined into _BeginFrameImpl and _ResetFrame.
void PS4Context::_ResetDrawState() {
    mCachedShaderStages = static_cast<sce::Gnm::ActiveShaderStages>(-1);
    mCachedPrimitiveType = static_cast<sce::Gnm::PrimitiveType>(-1);
    auto& gfx = _ActiveGfxContext();
    gfx.setGsModeOff();
    mGsModeEnabled = false;
    gfx.setCbControl(sce::Gnm::kCbModeNormal, sce::Gnm::kRasterOpCopy);
    mCbEnabled = true;
}

// Pipeline state -------------------------------------------------------------

// Reconstructed from eboot.elf at 0x8E8850. Forgets SetupDraw's state,
// turns the GS mode off and color writes on, unbinds every target, and resets
// blending, depth-stencil, raster and color-write state and the shader
// resources to their defaults.
void PS4Context::_BeginFrameImpl(unsigned int) {
    _ResetDrawState();

    RenderTargetParams params;
    PS4Context::_SetRenderTargetsImpl(kTargetModeNone, params);
    PS4Context::_SetBlendModeImpl(RndBlendMode::kSource, Hmx::Color::GetWhite());

    mDepthMode = 0;
    mStencilMode = 0;
    mStencilReference = 0;
    mStencilReadMask = 0xFF;
    mStencilWriteMask = 0xFF;
    _SyncDepthStencilControl();
    mFrontFace = 1;
    mCullMode = kCullNone;
    mFillMode = 1;
    _SyncPrimitiveSetup();
    _ActiveGfxContext().setRenderTargetMask(0xFFFF);
    mColorWriteTargets = 0xF;
    mColorWriteChannels = kWriteRGBA;

    PS4Context::_DeselectAllReadWriteTexturesImpl(kAllShaderStages);
    PS4Context::_DeselectAllSourceTexturesImpl(kAllShaderStages);
}

// Reconstructed from eboot.elf at 0x8E8D20. Binds the color and depth
// targets of the binding and its viewport, then clears the targets it marks:
// colors with the clear compute shader, after flushing the color metadata
// once, and depth and stencil through _ClearDepthStencil. After any clear
// the graphics pipe waits for the compute work and flushes the caches.
void PS4Context::_SetRenderTargetsImpl(
    RndTargetMode mode,
    const RenderTargetParams& params) {
    const sce::Gnm::RenderTarget* colorTargets[kColorRenderTargetCount] = {};
    const sce::Gnm::DepthRenderTarget* depthTarget = nullptr;
    const auto& targets = params.mTargets;
    if (mode == kTargetMode2D) {
        for (unsigned long index = 0; index < targets.mSize; ++index) {
            auto* texture = targets.mData[index].mTexture;
            const int type = texture->_GetTypeImpl();
            if (type == RndTextureBase::kTextureArray2D) {
                colorTargets[index] = static_cast<PS4TextureArray2D*>(texture)
                                          ->GetRenderTarget(targets.mData[index].mSlice);
            } else if (type == RndTextureBase::kTexture2D) {
                colorTargets[index] = static_cast<PS4Texture2D*>(texture)->GetRenderTarget();
            }
        }
        if (auto* texture = params.mDepthTexture) {
            const int type = texture->_GetTypeImpl();
            if (type == RndTextureBase::kTextureArray2D) {
                depthTarget = static_cast<PS4TextureArray2D*>(texture)
                                  ->GetDepthStencilTarget(params.mDepthSlice);
            } else if (type == RndTextureBase::kTexture2D) {
                depthTarget = static_cast<PS4Texture2D*>(texture)->GetDepthStencilTarget();
            }
        }
    } else if (mode == kTargetModeCube) {
        for (unsigned long index = 0; index < targets.mSize; ++index) {
            auto* texture = targets.mData[index].mTexture;
            if (texture->_GetTypeImpl() == RndTextureBase::kTextureCube) {
                colorTargets[index] = static_cast<PS4TextureCube*>(texture)->GetRenderTarget();
            }
        }
        if (auto* texture = params.mDepthTexture) {
            if (texture->_GetTypeImpl() == RndTextureBase::kTextureCube) {
                depthTarget = static_cast<PS4TextureCube*>(texture)->GetDepthStencilTarget();
            }
        }
    }

    for (unsigned int slot = 0; slot < kColorRenderTargetCount; ++slot) {
        _ActiveGfxContext().setRenderTarget(slot, colorTargets[slot]);
    }
    _ActiveGfxContext().setDepthRenderTarget(depthTarget);
    const auto left = static_cast<std::uint32_t>(params.mViewportX);
    const auto top = static_cast<std::uint32_t>(params.mViewportY);
    _ActiveGfxContext().setupScreenViewport(
        left,
        top,
        static_cast<std::uint32_t>(std::max(1.0F, params.mViewportWidth) + params.mViewportX),
        static_cast<std::uint32_t>(std::max(1.0F, params.mViewportHeight) + params.mViewportY),
        1.0F,
        0.0F);

    bool cleared = false;
    for (unsigned long index = 0; index < targets.mSize; ++index) {
        if (targets.mData[index].mClearMode != 1) {
            continue;
        }
        if (!cleared) {
            _ActiveGfxContext().triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateCbMeta);
        }
        RndCShaderClearBuffer::Params clear;
        clear.mTexture = targets.mData[index].mTexture;
        clear.mClearValue = params.mClearColor;
        TheRndDevice()->mShaderMgr.mClearBufferCShader->Dispatch(*this, clear);
        cleared = true;
    }
    if (depthTarget != nullptr && params.mDepthClearMode == 1) {
        cleared = _ClearDepthStencil(*depthTarget, params.mDepthClear, params.mStencilClear) || cleared;
    }
    if (cleared) {
        _WaitForEndOfPipe(sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2);
    }
}

// Reconstructed from eboot.elf at 0x8E92D0. Decal-lit blending rebuilds the
// control for every target.
void PS4Context::_SetBlendModeImpl(RndBlendMode mode, const Hmx::Color&) {
    auto& gfx = _ActiveGfxContext();
    sce::Gnm::BlendControl control;
    if (mode != RndBlendMode::kDecalLitSourceAlpha) {
        PS4RenderStateUtl::InitBlendControl(control, mode);
    }
    for (unsigned int slot = 0; slot < kColorRenderTargetCount; ++slot) {
        if (mode == RndBlendMode::kDecalLitSourceAlpha) {
            PS4RenderStateUtl::InitBlendControl(control, mode);
        }
        gfx.setBlendControl(slot, control);
    }
}

// Reconstructed from eboot.elf at 0x8E96F0. Each selected target writes all
// four channels or only RGB; other channel sets write nothing.
void PS4Context::_SetColorWriteMaskImpl(unsigned char targets, RndWriteMaskChannelSet channels) {
    std::uint32_t channelMask = 0;
    if (channels == kWriteRGBA) {
        channelMask = 0xF;
    } else if (channels == kWriteRGB) {
        channelMask = 0x7;
    }

    std::uint32_t gnmMask = 0;
    for (std::size_t slot = 0; slot < kColorRenderTargetCount; ++slot) {
        if ((targets & (1U << slot)) != 0) {
            gnmMask |= channelMask << (slot * 4);
        }
    }
    _ActiveGfxContext().setRenderTargetMask(gnmMask);
    mColorWriteTargets = targets;
    mColorWriteChannels = channels;
}

// Inlined into the depth and stencil setters and the depth clear.
void PS4Context::_SyncDepthStencilControl() {
    sce::Gnm::DepthStencilControl depthStencil;
    PS4RenderStateUtl::InitDepthStencilControl(depthStencil, mDepthMode, mStencilMode);
    sce::Gnm::StencilControl stencil;
    PS4RenderStateUtl::InitStencilControl(
        stencil, mStencilMode, mStencilReference, mStencilReadMask, mStencilWriteMask);
    sce::Gnm::StencilOpControl stencilOps;
    PS4RenderStateUtl::InitStencilOpControl(stencilOps, mStencilMode);
    auto& gfx = _ActiveGfxContext();
    gfx.setDepthStencilControl(depthStencil);
    gfx.setStencil(stencil);
    gfx.setStencilOpControl(stencilOps);
}

// Inlined into the front-face, cull and fill setters.
void PS4Context::_SyncPrimitiveSetup() {
    sce::Gnm::PrimitiveSetup setup;
    PS4RenderStateUtl::InitPrimitiveSetup(setup, mFrontFace, mCullMode, mFillMode);
    _ActiveGfxContext().setPrimitiveSetup(setup);
}

// Reconstructed from eboot.elf at 0x8E9F60.
void PS4Context::_SetDepthModeImpl(unsigned int mode) {
    mDepthMode = mode;
    _SyncDepthStencilControl();
}

// Reconstructed from eboot.elf at 0x8EA030. The masks are indices into the
// stencil mask table.
void PS4Context::_SetStencilModeImpl(
    unsigned int mode,
    unsigned char reference,
    unsigned int readMask,
    unsigned int writeMask) {
    mStencilMode = mode;
    mStencilReference = reference;
    mStencilReadMask = StencilMask(readMask);
    mStencilWriteMask = StencilMask(writeMask);
    _SyncDepthStencilControl();
}

// Reconstructed from eboot.elf at 0x8EA150.
void PS4Context::_SetFrontFaceImpl(bool counterClockwise) {
    mFrontFace = counterClockwise;
    _SyncPrimitiveSetup();
}

// Reconstructed from eboot.elf at 0x8EA1C0.
void PS4Context::_SetCullModeImpl(RndCullMode mode) {
    mCullMode = mode;
    _SyncPrimitiveSetup();
}

// Reconstructed from eboot.elf at 0x8EA230.
void PS4Context::_SetFillModeImpl(bool solid) {
    mFillMode = solid;
    _SyncPrimitiveSetup();
}

// Reconstructed from eboot.elf at 0x8EA2A0, 0x8EA2B0, 0x8EA2C0, and 0x8EA730:
// the PS4 context ignores these states.
void PS4Context::_SetDepthClipEnabledImpl(bool) {}
void PS4Context::_SetDepthBiasEnabledImpl(bool) {}
void PS4Context::_SetThickLinesImpl(bool) {}
void PS4Context::_DrawIndirectImpl(RndPrimitive, const RndComputeBuffer&) {}

// Draws ----------------------------------------------------------------------

// Reconstructed from eboot.elf at 0x8EA2D0.
void PS4Context::_DrawPrimitivesImpl(
    RndPrimitive primitive,
    RndVertexType type,
    const void* vertices,
    unsigned long count) {
    auto& transient = mTransientBuffers[mActiveFrame][type];
    auto& gfx = _ActiveGfxContext();
    const auto firstVertex = transient.Write(vertices, count);
    transient.Bind(gfx);
    gfx.setVertexBuffers(
        sce::Gnm::kShaderStageVs,
        RndVertexInterpreter::kNumStreams,
        PS4RenderUtl::kNumInstanceStreams,
        gPS4Device->mIdentityInstanceDescs);
    auto* indices = static_cast<std::uint16_t*>(gfx.allocateFromCommandBuffer(
        static_cast<std::uint32_t>(count * sizeof(std::uint16_t)),
        sce::Gnm::kEmbeddedDataAlignment4));
    for (std::size_t index = 0; index < count; ++index) {
        indices[index] = static_cast<std::uint16_t>(firstVertex + index);
    }

    gfx.setIndexSize(sce::Gnm::kIndexSize16);
    SetupDraw(primitive);
    gfx.drawIndex(static_cast<std::uint32_t>(count), indices);
}

// Reconstructed from eboot.elf at 0x8EA560.
void PS4Context::SetupDraw(RndPrimitive primitive) {
    const auto programStages = mActiveShaderStages;
    sce::Gnm::ActiveShaderStages stages;
    if ((programStages & kTessellationStageBits) != 0) {
        stages = (programStages & kGeometryStageBit) != 0
            ? sce::Gnm::kActiveShaderStagesLsHsEsGsVsPs
            : sce::Gnm::kActiveShaderStagesLsHsVsPs;
    } else {
        stages = (programStages & kGeometryStageBit) != 0
            ? sce::Gnm::kActiveShaderStagesEsGsVsPs
            : sce::Gnm::kActiveShaderStagesVsPs;
    }
    if (mCachedShaderStages != stages) {
        _ActiveGfxContext().setActiveShaderStages(stages);
        mCachedShaderStages = stages;
    }

    if ((mActiveShaderStages & kGeometryStageBit) == 0 && mGsModeEnabled) {
        _ActiveGfxContext().setGsModeOff();
        mGsModeEnabled = false;
    }

    const auto primitiveType = PS4RenderUtl::GetPrimitiveType(primitive);
    if (mCachedPrimitiveType != primitiveType) {
        _ActiveGfxContext().setPrimitiveType(primitiveType);
        mCachedPrimitiveType = primitiveType;
    }
}

// Reconstructed from eboot.elf at 0x8EA7D0.
void PS4Context::SetCbEnabled(bool enabled) {
    if (enabled != mCbEnabled) {
        _ActiveGfxContext().setCbControl(
            enabled ? sce::Gnm::kCbModeNormal : sce::Gnm::kCbModeDisable,
            sce::Gnm::kRasterOpCopy);
        mCbEnabled = enabled;
    }
}

// Shader resources -----------------------------------------------------------

// Reconstructed from eboot.elf at 0x8E9810. Clears all 128 read-write
// texture slots of each selected stage; only the graphics pipe records it.
void PS4Context::_DeselectAllReadWriteTexturesImpl(unsigned int stages) {
    if (mActivePipe != 0) {
        return;
    }
    auto& gfx = _ActiveGfxContext();
    for (unsigned int index = 0; index < kNumShaderStages; ++index) {
        if ((stages & (1U << index)) != 0) {
            gfx.setRwTextures(kGnmShaderStages[index], 0, kNumRwTextureSlots, nullptr);
        }
    }
}

// Reconstructed from eboot.elf at 0x8E9940. The map has
// _DeselectAllSourceTexturesImpl(unsigned int, unsigned long const*). Clears
// the first 16 texture and buffer slots of each selected stage.
void PS4Context::_DeselectAllSourceTexturesImpl(unsigned int stages) {
    if (mActivePipe != 0) {
        return;
    }
    for (unsigned int index = 0; index < kNumShaderStages; ++index) {
        if ((stages & (1U << index)) != 0) {
            const auto stage = kGnmShaderStages[index];
            _ActiveGfxContext().setTextures(stage, 0, kNumSourceSlots, nullptr);
            _ActiveGfxContext().setBuffers(stage, 0, kNumSourceSlots, nullptr);
        }
    }
}

// Reconstructed from eboot.elf at 0x8EA740. Copies the source's append
// counter from GDS into the destination's active storage.
void PS4Context::_CopyBufferCounter(
    const RndComputeBuffer& source,
    const RndComputeBuffer& dest) {
    const auto& ps4Source = static_cast<const PS4ComputeBuffer&>(source);
    const auto& ps4Dest = static_cast<const PS4ComputeBuffer&>(dest);
    auto& gfx = _ActiveGfxContext();
    gfx.setRwBuffers(sce::Gnm::kShaderStageCs, 0, 1, &ps4Source.ActiveBuffer());
    gfx.readAppendConsumeCounters(ps4Dest.ActiveStorage(), 0, 0, 1);
    gfx.setRwBuffers(sce::Gnm::kShaderStageCs, 0, 1, nullptr);
}

// Reconstructed from eboot.elf at 0x8EA830. Hull, domain and geometry
// stages take no sampler.
void PS4Context::_SetSamplerImpl(
    RndShaderProgramType type,
    unsigned int slot,
    unsigned int wrap,
    unsigned int filter) {
    sce::Gnm::Sampler sampler;
    PS4RenderStateUtl::InitSampler(
        sampler, static_cast<PS4RenderStateUtl::WrapMode>(wrap), filter);
    switch (type) {
    case kShaderProgramCompute:
        if (mActivePipe == 1) {
            _ActiveComputeContext().setSamplers(static_cast<int>(slot), 1, &sampler);
        } else if (mActivePipe == 0) {
            _ActiveGfxContext().setSamplers(sce::Gnm::kShaderStageCs, slot, 1, &sampler);
        }
        break;
    case kShaderProgramPixel:
        _ActiveGfxContext().setSamplers(sce::Gnm::kShaderStagePs, slot, 1, &sampler);
        break;
    case kShaderProgramVertex:
        _ActiveGfxContext().setSamplers(sce::Gnm::kShaderStageVs, slot, 1, &sampler);
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x8EA920. Clearing the pixel shader also
// turns color-buffer writes off.
void PS4Context::_DeactivateShaderProgramTypeImpl(RndShaderProgramType type) {
    auto& gfx = _ActiveGfxContext();
    switch (type) {
    case kShaderProgramVertex:
        gfx.setVsShader(nullptr, 0, nullptr);
        break;
    case kShaderProgramGeometry:
        gfx.setGsVsShaders(nullptr);
        break;
    case kShaderProgramPixel:
        gfx.setPsShader(nullptr);
        SetCbEnabled(false);
        break;
    case kShaderProgramCompute:
        gfx.setCsShader(nullptr);
        break;
    default:
        break;
    }
}

// Resource barriers ----------------------------------------------------------

// Inlined into _SetRenderTargetsImpl and _ResourceBarrierImpl.
void PS4Context::_WaitForEndOfPipe(std::uint32_t cacheActions) {
    auto& dcb = _ActiveGfxContext().m_dcb;
    auto* label = static_cast<std::uint32_t*>(
        dcb.allocateFromCommandBuffer(sizeof(std::uint32_t), sce::Gnm::kEmbeddedDataAlignment4));
    *label = 0;
    dcb.writeAtEndOfPipe(
        sce::Gnm::kEopCsDone,
        sce::Gnm::kEventWriteDestMemory,
        label,
        sce::Gnm::kEventWriteSource32BitsImmediate,
        1,
        static_cast<sce::Gnm::CacheAction>(cacheActions),
        sce::Gnm::kCachePolicyLru);
    dcb.waitOnAddress(label, 0xFFFFFFFF, sce::Gnm::kWaitCompareFuncEqual, 1);
}

// Inlined into _ResourceBarrierImpl. A subresource selects one slice of an
// array; -1 views every slice.
void PS4Context::_DecompressColorTarget(
    const RndResourceBarrier& barrier,
    std::uint32_t cacheActions) {
    auto* texture = static_cast<RndTextureBase*>(barrier.mResource);
    sce::Gnm::RenderTarget target;
    const int type = texture->_GetTypeImpl();
    if (type == RndTextureBase::kTextureArray2D) {
        auto* array = static_cast<PS4TextureArray2D*>(texture);
        target = *array->GetRenderTarget(static_cast<unsigned long>(-1));
        if (barrier.mSubresource == static_cast<std::uint64_t>(-1)) {
            target.setArrayView(0, static_cast<std::uint32_t>(array->mBaseDesc.mArraySize) - 1);
        } else {
            const auto slice = static_cast<std::uint32_t>(
                barrier.mSubresource / (array->_GetNumMipsImpl() + 1));
            target.setArrayView(slice, slice);
        }
    } else if (type == RndTextureBase::kTexture2D) {
        target = *static_cast<PS4Texture2D*>(texture)->GetRenderTarget();
    }

    auto& gfx = _ActiveGfxContext();
    const auto slices = target.getLastArraySliceIndex() - target.getBaseArraySliceIndex() + 1;
    gfx.waitForGraphicsWrites(
        target.getBaseAddress256ByteBlocks(),
        (slices * target.getSliceSizeInBytes()) >> 8,
        sce::Gnm::kWaitTargetSlotCb0 | sce::Gnm::kWaitTargetSlotCb1 | sce::Gnm::kWaitTargetSlotCb2 |
            sce::Gnm::kWaitTargetSlotCb3 | sce::Gnm::kWaitTargetSlotCb4 | sce::Gnm::kWaitTargetSlotCb5 |
            sce::Gnm::kWaitTargetSlotCb6 | sce::Gnm::kWaitTargetSlotCb7,
        static_cast<sce::Gnm::CacheAction>(cacheActions),
        sce::Gnm::kExtendedCacheActionFlushAndInvalidateCbCache,
        sce::Gnm::kStallCommandBufferParserDisable);
    gfx.triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateCbPixelData);
    if (target.getDccCompressionEnable()) {
        sce::Gnmx::decompressDccSurface(&gfx, &target);
    } else {
        sce::Gnmx::eliminateFastClear(&gfx, &target);
        sce::Gnmx::decompressFmaskSurface(&gfx, &target);
    }
}

// Inlined into _ResourceBarrierImpl.
void PS4Context::_DecompressDepthTarget(
    const RndResourceBarrier& barrier,
    std::uint32_t cacheActions) {
    auto* texture = static_cast<RndTextureBase*>(barrier.mResource);
    sce::Gnm::DepthRenderTarget target;
    const int type = texture->_GetTypeImpl();
    if (type == RndTextureBase::kTextureArray2D) {
        auto* array = static_cast<PS4TextureArray2D*>(texture);
        target = *array->GetDepthStencilTarget(static_cast<unsigned long>(-1));
        if (barrier.mSubresource == static_cast<std::uint64_t>(-1)) {
            target.setArrayView(0, static_cast<std::uint32_t>(array->mBaseDesc.mArraySize) - 1);
        } else {
            const auto slice = static_cast<std::uint32_t>(
                barrier.mSubresource / (array->_GetNumMipsImpl() + 1));
            target.setArrayView(slice, slice);
        }
    } else if (type == RndTextureBase::kTexture2D) {
        target = *static_cast<PS4Texture2D*>(texture)->GetDepthStencilTarget();
    }

    auto& gfx = _ActiveGfxContext();
    const auto slices = target.getLastArraySliceIndex() - target.getBaseArraySliceIndex() + 1;
    gfx.waitForGraphicsWrites(
        target.getZWriteAddress256ByteBlocks(),
        (slices * target.getZSliceSizeInBytes()) >> 8,
        sce::Gnm::kWaitTargetSlotDb,
        static_cast<sce::Gnm::CacheAction>(cacheActions),
        sce::Gnm::kExtendedCacheActionFlushAndInvalidateDbCache,
        sce::Gnm::kStallCommandBufferParserDisable);
    if (target.getHtileTextureCompatible()) {
        gfx.triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateDbMeta);
    }
    sce::Gnmx::decompressDepthSurface(&gfx, &target);
}

// Reconstructed from eboot.elf at 0x8EAA10. Immediate barriers accumulate
// cache actions and wait once at the end; begin barriers signal a shared
// label that the matching end barriers wait for. Leaving render-target or
// depth-write state decompresses the target on the graphics ring, signalling
// from there when recording compute.
void PS4Context::_ResourceBarrierImpl(unsigned long count, const RndResourceBarrier* barriers) {
    volatile std::uint32_t* sharedLabel = nullptr;
    if (count == 0) {
        return;
    }
    std::uint32_t cacheActions = 0;
    bool waitForCompletion = false;
    for (unsigned long index = 0; index < count; ++index) {
        const auto& barrier = barriers[index];
        if (barrier.mType == RndResourceBarrierType::kUnorderedAccess) {
            switch (barrier.mPhase) {
            case RndResourceBarrierPhase::kEnd:
                _WaitForResource(barrier.mResource);
                break;
            case RndResourceBarrierPhase::kBegin:
                _SignalResource(barrier.mResource, sharedLabel);
                cacheActions |= sce::Gnm::kCacheActionInvalidateL1;
                break;
            case RndResourceBarrierPhase::kImmediate:
                cacheActions |= sce::Gnm::kCacheActionInvalidateL1;
                waitForCompletion = true;
                break;
            }
            continue;
        }
        if (barrier.mType != RndResourceBarrierType::kTransition ||
            barrier.mBefore == barrier.mAfter) {
            continue;
        }

        // A back buffer about to be rendered to waits until it is no longer
        // being presented.
        std::uint32_t barrierCacheActions = sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2;
        switch (barrier.mAfter) {
        case RndResourceState::kRenderTarget: {
            auto* texture = static_cast<RndTextureBase*>(barrier.mResource);
            if (texture->_GetTypeImpl() == RndTextureBase::kTexture2D) {
                auto* pending = static_cast<PS4Texture2D*>(texture)->mPendingPresentations;
                if (pending != nullptr) {
                    auto* window = static_cast<PS4Window*>(gPS4Device->mMainWindow);
                    _ActiveGfxContext().waitOnAddress(
                        pending + window->mActiveBuffer,
                        0xFFFFFFFF,
                        sce::Gnm::kWaitCompareFuncEqual,
                        0);
                }
            }
            barrierCacheActions = sce::Gnm::kCacheActionNone;
            break;
        }
        case RndResourceState::kDepthWrite:
        case RndResourceState::kResolveDestination:
            barrierCacheActions = sce::Gnm::kCacheActionNone;
            break;
        default:
            break;
        }

        switch (barrier.mBefore) {
        case RndResourceState::kRenderTarget:
        case RndResourceState::kDepthWrite: {
            if (barrier.mPhase == RndResourceBarrierPhase::kEnd) {
                _WaitForResource(barrier.mResource);
                continue;
            }
            if (barrier.mBefore == RndResourceState::kRenderTarget) {
                _DecompressColorTarget(barrier, barrierCacheActions);
            } else {
                _DecompressDepthTarget(barrier, barrierCacheActions);
            }
            _ResetDrawState();
            volatile std::uint32_t* label = nullptr;
            const auto pipeline = mActivePipe;
            if (pipeline == kPipelineGraphics) {
                if (barrier.mPhase == RndResourceBarrierPhase::kBegin) {
                    _SignalResource(barrier.mResource, label);
                } else {
                    waitForCompletion = true;
                }
            } else {
                const auto slot = mActiveComputeSlot;
                SetActivePipeline(kPipelineGraphics, 0);
                _SignalResource(barrier.mResource, label);
                const auto phase = barrier.mPhase;
                SetActivePipeline(static_cast<RndPipeline>(pipeline), slot);
                if (phase != RndResourceBarrierPhase::kBegin) {
                    _WaitForResource(barrier.mResource);
                }
            }
            continue;
        }
        case RndResourceState::kUnorderedAccess:
        case RndResourceState::kCopyDestination:
            break;
        case RndResourceState::kResolveDestination:
            continue;
        default:
            // From a read state only a write needs a barrier, and it needs no
            // cache action.
            switch (barrier.mAfter) {
            case RndResourceState::kRenderTarget:
            case RndResourceState::kUnorderedAccess:
            case RndResourceState::kDepthWrite:
            case RndResourceState::kStreamOutput:
            case RndResourceState::kCopyDestination:
            case RndResourceState::kResolveDestination:
                barrierCacheActions = sce::Gnm::kCacheActionNone;
                break;
            default:
                continue;
            }
            break;
        }

        switch (barrier.mPhase) {
        case RndResourceBarrierPhase::kEnd:
            _WaitForResource(barrier.mResource);
            break;
        case RndResourceBarrierPhase::kBegin:
            _SignalResource(barrier.mResource, sharedLabel);
            cacheActions |= barrierCacheActions;
            break;
        case RndResourceBarrierPhase::kImmediate:
            cacheActions |= barrierCacheActions;
            waitForCompletion = true;
            break;
        }
    }

    if (waitForCompletion) {
        if (mActivePipe == kPipelineCompute) {
            _ActiveComputeContext().m_dcb.triggerEvent(sce::Gnm::kEventTypeCsPartialFlush);
        } else if (mActivePipe == kPipelineGraphics) {
            _WaitForEndOfPipe(cacheActions | sce::Gnm::kCacheActionWriteBackAndInvalidateL1andL2);
            return;
        }
    }
    if (cacheActions != 0) {
        if (mActivePipe == kPipelineCompute) {
            _ActiveComputeContext().m_dcb.flushShaderCachesAndWait(
                static_cast<sce::Gnm::CacheAction>(cacheActions), 0);
        } else if (mActivePipe == kPipelineGraphics) {
            _ActiveGfxContext().waitForGraphicsWrites(
                0,
                1,
                0,
                static_cast<sce::Gnm::CacheAction>(cacheActions),
                0,
                sce::Gnm::kStallCommandBufferParserDisable);
        }
    }
}

// Reconstructed from eboot.elf at 0x8E7EB0. Labels are handed out in a ring.
// When the ring would reach the first label of the previous frame, it is
// replaced by one twice the size. A new frame records where it starts; two
// frames on, every label is free again, and one frame on only the signals of
// the last frame are kept.
volatile std::uint32_t* PS4Context::_AllocateResourceLabel() {
    auto frame = static_cast<unsigned long>(gPS4Device->mFrameCount);
    if (mLabelFrame != frame) {
        if (mLabelFrame + 1 == frame) {
            auto keep = mResourceSignals.end();
            while (keep != mResourceSignals.begin()) {
                if (frame - static_cast<unsigned long>((keep - 1)->mRenderEpoch) > 1) {
                    break;
                }
                --keep;
            }
            mResourceSignals.erase(mResourceSignals.begin(), keep);
        } else {
            mNextLabel = 0;
            if (mLabels != nullptr) {
                mLabelFrameStarts[0] = mLabelCapacity - 1;
                mLabelFrameStarts[1] = mLabelCapacity - 1;
            } else {
                mLabelFrameStarts[0] = 0;
                mLabelFrameStarts[1] = 0;
            }
        }
        mLabelFrame = frame;
        mLabelFrameStarts[frame & 1] = mNextLabel;
        frame = mLabelFrame;
    }

    auto* labels = mLabels;
    if (mNextLabel == mLabelFrameStarts[(frame & 1) == 0 ? 1 : 0]) {
        if (labels != nullptr) {
            gPS4Device->DeferredDelete(labels);
        }
        const auto capacity = mLabelCapacity;
        mLabelCapacity = 2 * capacity;
        mNextLabel = 0;
        mLabelFrameStarts[0] = 2 * capacity - 1;
        mLabelFrameStarts[1] = 2 * capacity - 1;
        labels = static_cast<std::uint32_t*>(MemAlloc(8 * capacity, "labels", 0));
        mLabels = labels;
    }
    auto* label = labels + mNextLabel;
    const auto next = mNextLabel + 1;
    mNextLabel = next != mLabelCapacity ? next : 0;
    return label;
}

// Reconstructed from eboot.elf at 0x8EB3E0. A barrier's first resource
// allocates the shared label and signals it at end of pipe; every resource
// is recorded against it.
void PS4Context::_SignalResource(
    const void* resource,
    volatile std::uint32_t*& sharedLabel) {
    if (sharedLabel == nullptr) {
        auto* label = _AllocateResourceLabel();
        sharedLabel = label;
        *label = 0;
        if (mActivePipe == 1) {
            _ActiveComputeContext().writeReleaseMemEvent(
                sce::Gnm::kReleaseMemEventCsDone,
                sce::Gnm::kEventWriteDestMemory,
                const_cast<std::uint32_t*>(label),
                sce::Gnm::kEventWriteSource32BitsImmediate,
                kResourceReadyValue,
                sce::Gnm::kCacheActionNone,
                sce::Gnm::kCachePolicyLru);
        } else if (mActivePipe == 0) {
            _ActiveGfxContext().writeAtEndOfPipe(
                sce::Gnm::kEopCbDbReadsDone,
                sce::Gnm::kEventWriteDestMemory,
                const_cast<std::uint32_t*>(label),
                sce::Gnm::kEventWriteSource32BitsImmediate,
                kResourceReadyValue,
                sce::Gnm::kCacheActionNone,
                sce::Gnm::kCachePolicyLru);
        }
    }

    ResourceSignal signal;
    signal.mResource = resource;
    signal.mLabel = sharedLabel;
    signal.mRenderEpoch = gPS4Device->mFrameCount;
    mResourceSignals.push_back(signal);
}

// Reconstructed from eboot.elf at 0x8EB590. Waits for the first signal of the
// resource, then forgets every resource that shared its label.
void PS4Context::_WaitForResource(const void* resource) {
    auto it = mResourceSignals.begin();
    while (it != mResourceSignals.end() && it->mResource != resource) {
        ++it;
    }
    if (it == mResourceSignals.end()) {
        return;
    }

    auto* label = const_cast<std::uint32_t*>(it->mLabel);
    if (mActivePipe == 0) {
        _ActiveGfxContext().waitOnAddress(
            label, 0xFFFFFFFF, sce::Gnm::kWaitCompareFuncEqual, kResourceReadyValue);
    } else if (mActivePipe == 1) {
        _ActiveComputeContext().waitOnAddress(
            label, 0xFFFFFFFF, sce::Gnm::kWaitCompareFuncEqual, kResourceReadyValue);
    }

    const volatile std::uint32_t* signalled = it->mLabel;
    mResourceSignals.erase(
        std::remove_if(
            mResourceSignals.begin(),
            mResourceSignals.end(),
            [signalled](const ResourceSignal& signal) { return signal.mLabel == signalled; }),
        mResourceSignals.end());
}

// Fences, dispatch and markers -----------------------------------------------

// Reconstructed from eboot.elf at 0x8EB730.
void PS4Context::_SignalFenceImpl(RndFence& fence) {
    auto& ps4Fence = static_cast<PS4Fence&>(fence);
    if (mActivePipe == 1) {
        auto* address = ps4Fence.mLabel;
        _ActiveComputeContext().writeReleaseMemEvent(
            sce::Gnm::kReleaseMemEventCsDone,
            sce::Gnm::kEventWriteDestMemory,
            const_cast<std::uint32_t*>(address),
            sce::Gnm::kEventWriteSource32BitsImmediate,
            ps4Fence.NextValue(),
            sce::Gnm::kCacheActionNone,
            sce::Gnm::kCachePolicyLru);
    } else if (mActivePipe == 0) {
        auto* address = ps4Fence.mLabel;
        _ActiveGfxContext().writeAtEndOfPipe(
            sce::Gnm::kEopCbDbReadsDone,
            sce::Gnm::kEventWriteDestMemory,
            const_cast<std::uint32_t*>(address),
            sce::Gnm::kEventWriteSource32BitsImmediate,
            ps4Fence.NextValue(),
            sce::Gnm::kCacheActionNone,
            sce::Gnm::kCachePolicyLru);
    }
}

// Reconstructed from eboot.elf at 0x8EB7F0.
void PS4Context::_WaitFenceImpl(const RndFence& fence) {
    const auto& ps4Fence = static_cast<const PS4Fence&>(fence);
    auto* address = const_cast<std::uint32_t*>(ps4Fence.mLabel);
    if (mActivePipe == 1) {
        _ActiveComputeContext().waitOnAddress(
            address, 0xFFFFFFFF, sce::Gnm::kWaitCompareFuncGreaterEqual, ps4Fence.mSequence);
    } else if (mActivePipe == 0) {
        _ActiveGfxContext().waitOnAddress(
            address, 0xFFFFFFFF, sce::Gnm::kWaitCompareFuncGreaterEqual, ps4Fence.mSequence);
    }
}

// Reconstructed from eboot.elf at 0x8EB870. The SDK's inline dispatch
// prepares and finishes the CUE state around the packet.
void PS4Context::_DispatchComputeImpl(unsigned int x, unsigned int y, unsigned int z) {
    if (mActivePipe == 1) {
        _ActiveComputeContext().dispatch(x, y, z);
    } else if (mActivePipe == 0) {
        _ActiveGfxContext().dispatch(x, y, z);
    }
}

// Reconstructed from eboot.elf at 0x8EB970.
void PS4Context::_PushMarkerImpl(const char* name) {
    if (mActivePipe == 1) {
        _ActiveComputeContext().pushMarker(name, kDebugMarkerColor);
    } else if (mActivePipe == 0) {
        _ActiveGfxContext().pushMarker(name, kDebugMarkerColor);
    }
}

// Reconstructed from eboot.elf at 0x8EB9D0.
void PS4Context::_PopMarkerImpl() {
    if (mActivePipe == 1) {
        _ActiveComputeContext().popMarker();
    } else if (mActivePipe == 0) {
        _ActiveGfxContext().popMarker();
    }
}

// GPU statistics -------------------------------------------------------------

// The end-of-pipe event of a timestamp: CB/DB flushes on the graphics pipe,
// compute completion otherwise. The timestamps are always written by the
// graphics context. Name not in the reference map.
sce::Gnm::EndOfPipeEventType PS4Context::_GpuTimestampEvent() const {
    return mActivePipe == 0 ? sce::Gnm::kEopFlushCbDbCaches : sce::Gnm::kEopCsDone;
}

// Reconstructed from eboot.elf at 0x8EBA20.
void PS4Context::_BeginGpuStatsImpl(unsigned long key) {
    auto& block = mGpuStatBlocks[mNextGpuStatBlock];
    const auto next = mNextGpuStatBlock + 1;
    mNextGpuStatBlock = next > kNumGpuStatBlocks - 1 ? 0 : next;
    block.mActive = true;
    _ActiveGfxContext().writeAtEndOfPipe(
        _GpuTimestampEvent(),
        sce::Gnm::kEventWriteDestMemory,
        const_cast<std::uint64_t*>(block.mBegin),
        sce::Gnm::kEventWriteSourceGpuCoreClockCounter,
        0,
        sce::Gnm::kCacheActionNone,
        sce::Gnm::kCachePolicyLru);
    mGpuStats[key] = &block;
}

// Reconstructed from eboot.elf at 0x8EBBB0.
void PS4Context::_EndGpuStatsImpl(unsigned long key) {
    auto* block = mGpuStats.find(key)->second;
    _ActiveGfxContext().writeAtEndOfPipe(
        _GpuTimestampEvent(),
        sce::Gnm::kEventWriteDestMemory,
        const_cast<std::uint64_t*>(block->mEnd),
        sce::Gnm::kEventWriteSourceGpuCoreClockCounter,
        0,
        sce::Gnm::kCacheActionNone,
        sce::Gnm::kCachePolicyLru);
}

// Reconstructed from eboot.elf at 0x8EBC70. The GPU core clock runs at
// 800 MHz.
RndGpuStatSample PS4Context::_EvalAndRetireGpuStatsImpl(unsigned long key) {
    const auto it = mGpuStats.find(key);
    auto* block = it->second;
    const auto elapsedTicks = *block->mEnd - *block->mBegin;

    RndGpuStatSample statistics = {};
    statistics.mSeconds = static_cast<float>(elapsedTicks) * kGpuClockSeconds;

    block->mActive = false;
    mGpuStats.erase(it);
    return statistics;
}

// Reconstructed from eboot.elf at 0x8EBDA0.
void PS4Context::_FlushClear() {
    RndShaderBasic::Params basic;
    gPS4Device->mShaderMgr.mBasicShader->Select(*this, basic);
    SetCbEnabled(false);
    _ActiveGfxContext().setPsShader(nullptr);

    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    quad.mKeepState = true;
    RndDrawUtl::DrawQuad2D(*this, quad);
    SetCbEnabled(true);
}

// Reconstructed from eboot.elf at 0x8E99E0. The compute clears cover every
// slice of the target's array view, 64 dwords per thread group.
bool PS4Context::_ClearDepthStencil(
    const sce::Gnm::DepthRenderTarget& target,
    float depth,
    unsigned char stencil) {
    if (target.getHtileAccelerationEnable()) {
        _ActiveGfxContext().triggerEvent(sce::Gnm::kEventTypeFlushAndInvalidateDbMeta);
        const auto baseSlice = target.getBaseArraySliceIndex();
        const auto numSlices = target.getLastArraySliceIndex() - baseSlice + 1;
        if (target.getHtileStencilDisable() && target.getStencilWriteAddress() != nullptr) {
            RndCShaderClearBuffer::Params params;
            const auto value = static_cast<float>(stencil * 0x01010101U);
            params.mClearValue = Hmx::Color(value, value, value, value);
            const auto slot =
                TheRndDevice()->mShaderMgr.mClearBufferCShader->Select(*this, params);
            const auto sliceSize = target.getStencilSliceSizeInBytes();
            const auto dwords = numSlices * (sliceSize / 4);
            sce::Gnm::Buffer buffer;
            buffer.initAsDataBuffer(
                static_cast<std::uint8_t*>(target.getStencilWriteAddress()) + baseSlice * sliceSize,
                sce::Gnm::kDataFormatR32Uint,
                dwords);
            buffer.setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
            _ActiveGfxContext().setRwBuffers(
                sce::Gnm::kShaderStageCs, static_cast<std::uint32_t>(slot), 1, &buffer);
            _DispatchComputeImpl(dwords / 64 + (dwords / 64 * 64 < dwords ? 1 : 0), 1, 1);
        }

        RndCShaderClearBuffer::Params params;
        const auto slot = TheRndDevice()->mShaderMgr.mClearBufferCShader->Select(*this, params);
        const auto sliceSize = target.getHtileSliceSizeInBytes();
        const auto dwords = numSlices * (sliceSize / 4);
        sce::Gnm::Buffer buffer;
        buffer.initAsDataBuffer(
            static_cast<std::uint8_t*>(target.getHtileAddress()) + baseSlice * sliceSize,
            sce::Gnm::kDataFormatR32Uint,
            dwords);
        buffer.setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
        _ActiveGfxContext().setRwBuffers(
            sce::Gnm::kShaderStageCs, static_cast<std::uint32_t>(slot), 1, &buffer);
        _DispatchComputeImpl(dwords / 64 + (dwords / 64 * 64 < dwords ? 1 : 0), 1, 1);
        return true;
    }

    sce::Gnm::DbRenderControl renderControl;
    renderControl.init();
    renderControl.setDepthClearEnable(true);
    renderControl.setStencilClearEnable(true);
    _ActiveGfxContext().setDbRenderControl(renderControl);
    sce::Gnm::DepthStencilControl depthStencil;
    depthStencil.init();
    depthStencil.setDepthControl(sce::Gnm::kDepthControlZWriteEnable, sce::Gnm::kCompareFuncAlways);
    depthStencil.setStencilFunction(sce::Gnm::kCompareFuncAlways);
    depthStencil.setDepthEnable(true);
    depthStencil.setStencilEnable(true);
    _ActiveGfxContext().setDepthStencilControl(depthStencil);
    sce::Gnm::StencilOpControl stencilOps;
    stencilOps.init();
    stencilOps.setStencilOps(
        sce::Gnm::kStencilOpReplaceTest,
        sce::Gnm::kStencilOpReplaceTest,
        sce::Gnm::kStencilOpReplaceTest);
    _ActiveGfxContext().setStencilOpControl(stencilOps);
    sce::Gnm::StencilControl stencilControl;
    stencilControl.m_testVal = 0xFF;
    stencilControl.m_mask = 0xFF;
    stencilControl.m_writeMask = 0xFF;
    stencilControl.m_opVal = 0xFF;
    _ActiveGfxContext().setStencil(stencilControl);
    _ActiveGfxContext().setDepthClearValue(depth);
    _ActiveGfxContext().setStencilClearValue(stencil);
    _ActiveGfxContext().setRenderTargetMask(0);
    _FlushClear();

    renderControl.init();
    renderControl.setDepthTileWriteBackPolicy(sce::Gnm::kDbTileWriteBackPolicyCompressionForbidden);
    _ActiveGfxContext().setDbRenderControl(renderControl);
    _SetColorWriteMaskImpl(
        static_cast<unsigned char>(mColorWriteTargets), mColorWriteChannels);
    _SyncDepthStencilControl();
    return false;
}
