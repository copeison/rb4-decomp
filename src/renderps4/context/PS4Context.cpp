#include "renderps4/context/PS4Context.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "os/memory/MemMgr.h"
#include "render/context/RndResourceBarrier.h"
#include "render/system/render_runtime_adapters.h"
#include "render/meshes/RndMesh.h"
#include "render/meshes/RndVertexInterpreter.h"
#include "render/shaders/RndShaderEnums.h"
#include "render/system/RndDevice.h"
#include "renderps4/buffers/PS4ComputeBuffer.h"
#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4Fence.h"
#include "renderps4/system/PS4RenderUtl.h"

// Render-target and depth-target queries the context uses. They have no
// original home yet; not yet reconstructed. Names not in the reference map.
const RndContext::RenderTargetParams& orbis_default_render_target_binding();
const RndContext::BlendParams& orbis_default_blend_configuration();
std::size_t orbis_render_target_color_count(
    const RndContext::RenderTargetParams& binding);
const sce::Gnm::RenderTarget* orbis_resolve_color_render_target(
    const RndContext::RenderTargetParams& binding,
    std::int32_t target_kind,
    std::size_t slot);
const sce::Gnm::DepthRenderTarget* orbis_resolve_depth_render_target(
    const RndContext::RenderTargetParams& binding,
    std::int32_t target_kind);
PS4Context::ViewportRect orbis_render_target_viewport(
    const RndContext::RenderTargetParams& binding);
bool orbis_color_render_target_requires_sync(
    const RndContext::RenderTargetParams& binding,
    std::size_t slot);
bool orbis_depth_render_target_requires_prepare(
    const RndContext::RenderTargetParams& binding);
bool orbis_depth_target_has_htile(
    const sce::Gnm::DepthRenderTarget& target);
bool orbis_depth_target_stencil_clear_range(
    const sce::Gnm::DepthRenderTarget& target,
    PS4Context::DepthClearRange& range);
PS4Context::DepthClearRange orbis_depth_target_htile_clear_range(
    const sce::Gnm::DepthRenderTarget& target);

namespace {

constexpr std::size_t kTransientVertexCapacity = 0x40000;
constexpr std::size_t kCueSlotCount = 64;
constexpr std::size_t kCueHeapBytesPerSlot = 39872;
constexpr std::size_t kDrawCommandBufferSize = 32 * 1024 * 1024;
constexpr std::size_t kResourceBufferSize = 2 * 1024 * 1024;
constexpr std::size_t kConstantUpdateBufferSize = 4 * 1024 * 1024;
constexpr std::size_t kScratchBufferSize = 4 * 1024 * 1024;
constexpr std::size_t kComputeCommandBufferSize = 0x3FFFFC;
constexpr std::size_t kComputeQueueRingSize = 4096;
constexpr std::size_t kComputeQueueRingAlignment = 256;
constexpr unsigned int kNumShaderStages = 6;
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
constexpr std::int32_t kUnboundTargetKind = -1;
constexpr std::array<std::uint8_t, 10> kStencilMasks = {
    0xFF, 0x07, 0x08, 0x10, 0x0F,
    0x1F, 0x20, 0x28, 0x30, 0xC0,
};

constexpr std::uint32_t kDebugMarkerColor = 0xFF0000FF;
constexpr float kGpuClockSeconds = 1.25e-9F;

constexpr std::uint32_t kInvalidateL1 = 0x10;
constexpr std::uint32_t kWriteBackAndInvalidateL1L2 = 0x38;
constexpr std::uint32_t kResourceReadyValue = 1;

std::uint8_t StencilMask(std::uint32_t index) {
    return index < kStencilMasks.size() ? kStencilMasks[index] : 0;
}

bool IsState(RndResourceState state, RndResourceState expected) {
    return state == expected;
}

bool IsWriteDestination(RndResourceState state) {
    return IsState(state, RndResourceState::kRenderTarget) ||
        IsState(state, RndResourceState::kUnorderedAccess) ||
        IsState(state, RndResourceState::kDepthWrite) ||
        IsState(state, RndResourceState::kStreamOutput) ||
        IsState(state, RndResourceState::kCopyDestination) ||
        IsState(state, RndResourceState::kResolveDestination);
}

std::uint32_t DestinationCacheActions(RndResourceState state) {
    if (IsState(state, RndResourceState::kUnorderedAccess) ||
        IsState(state, RndResourceState::kStreamOutput) ||
        IsState(state, RndResourceState::kCopyDestination)) {
        return kWriteBackAndInvalidateL1L2;
    }
    return 0;
}

}  // namespace

// Construction and submission ------------------------------------------------

PS4Context* PS4Context::_CreateImmediate(PS4Device& device) {
    auto* context = new PS4Context;
    device._InstallImmediateContext(context);
    return context;
}

// Reconstructed from eboot.elf at 0x8E72B0. The binary constructs the
// transient buffers (0x8EC7C0) after the state defaults; here the compiler
// constructs them as members before the body, since the earlier members are
// not yet modeled.
PS4Context::PS4Context()
    : RndContext(false),
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
    _InitCommandState();

    _InitStateDefaults();
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

    _InitComputeQueue(
        0, 1, 1, kComputeQueueRingSize, kComputeQueueRingAlignment);
    _InitComputeQueue(
        1, 0, 0, kComputeQueueRingSize, kComputeQueueRingAlignment);
    for (std::size_t slot = 0; slot < kComputeContextCount; ++slot) {
        _InitComputeContext(slot, kCueSlotCount, kComputeCommandBufferSize);
    }
    _InitLabelPool(kInitialLabelCapacity);
}

// Reconstructed from eboot.elf at 0x8E7AF0.
void PS4Context::_CreateGfxContext() {
    const auto cueHeapSize = kCueSlotCount * kCueHeapBytesPerSlot;
    for (std::size_t slot = 0; slot < kFrameSlotCount; ++slot) {
        _InitGfxSlot(
            slot,
            cueHeapSize,
            kDrawCommandBufferSize,
            kResourceBufferSize,
            kConstantUpdateBufferSize,
            kScratchBufferSize);
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

// Reconstructed from eboot.elf at 0x8E8070. The binary destroys the
// transient buffers (0x8EC7D0) after releasing the timestamp pool; here the
// compiler destroys them as members after the body, since the members around
// them are not yet modeled.
PS4Context::~PS4Context() {
    MemFree(mGpuTimestamps);
    _ReleaseLabelPool();
    _DestructCommandState();
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

// Reconstructed from eboot.elf at 0x8E82D0.
void PS4Context::SubmitFrame() {
    const auto frame = mActiveFrame;
    _EmitEndOfFrameEvent(frame);

    for (std::size_t slot = 0; slot < kComputeContextsPerFrame; ++slot) {
        mSubmissionPending[frame][slot + 1] = 1;
        _EmitComputeCompletion(frame, slot);

        const auto queue = slot < kHighPriorityComputeContextCount ? 0U : 1U;
        _SubmitCompute(frame, slot, queue);
    }

    mSubmissionPending[frame][0] = 1;
    _EmitGfxCompletion(frame);
    _SubmitGfx(frame);
    mActiveFrame = (frame + 1) % kFrameSlotCount;
}

// Reconstructed from eboot.elf at 0x8E8450.
void PS4Context::_ResetFrame() {
    const auto frame = mActiveFrame;
    _ResetGfxSlot(frame);
    _InitGfxHardwareState(frame);
    _ClearFrameDrawCount(frame);

    if (!mDisableComputeQueues) {
        for (std::size_t slot = 0; slot < kComputeContextsPerFrame; ++slot) {
            _ResetComputeSlot(frame, slot);
        }
    }

    _InitFrameCommandState(frame);
    for (std::size_t format = 0; format < kTransientFormatCount; ++format) {
        mTransientBuffers[frame][format].Reset();
    }
    _EmitDefaultControlState(frame);
}

// Pipeline state -------------------------------------------------------------

// Reconstructed from eboot.elf at 0x8E8850.
void PS4Context::_BeginFrameImpl() {
    _ResetCachedPipelineState();
    PS4Context::_SetRenderTargetsImpl(
        kUnboundTargetKind, orbis_default_render_target_binding());
    PS4Context::_SetBlendModeImpl(
        RndBlendMode::kSource, orbis_default_blend_configuration());
    _SetDefaultRasterState();
    _SetDefaultDepthStencilState();
    _DisableStreamOutput();
    _ClearShaderResources();
}

// Reconstructed from eboot.elf at 0x8E8D20.
void PS4Context::_SetRenderTargetsImpl(int mode, const RenderTargetParams& params) {
    std::array<const sce::Gnm::RenderTarget*, kColorRenderTargetCount>
        colorTargets{};
    const auto colorCount = std::min(
        orbis_render_target_color_count(params), colorTargets.size());
    for (std::size_t slot = 0; slot < colorCount; ++slot) {
        colorTargets[slot] =
            orbis_resolve_color_render_target(params, mode, slot);
    }
    const auto* depthTarget = orbis_resolve_depth_render_target(params, mode);

    for (std::size_t slot = 0; slot < colorTargets.size(); ++slot) {
        _BindColorTarget(slot, colorTargets[slot]);
    }
    _BindDepthTarget(depthTarget);
    _SetViewportAndScissor(orbis_render_target_viewport(params));

    bool synchronized = false;
    for (std::size_t slot = 0; slot < colorCount; ++slot) {
        if (!orbis_color_render_target_requires_sync(params, slot)) {
            continue;
        }
        if (!synchronized) {
            _BeginRenderTargetSync();
        }
        _PrepareColorTarget(params, slot);
        synchronized = true;
    }

    if (depthTarget != nullptr &&
        orbis_depth_render_target_requires_prepare(params)) {
        synchronized = _PrepareDepthTarget(*depthTarget, params) || synchronized;
    }
    if (synchronized) {
        _FinishRenderTargetSync();
    }
}

// Reconstructed from eboot.elf at 0x8E92D0. Decal-lit blending rebuilds the
// control for every target.
void PS4Context::_SetBlendModeImpl(RndBlendMode mode, const BlendParams&) {
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

// Reconstructed from eboot.elf at 0x8E9810. The map has
// _DeselectAllReadWriteTexturesImpl(unsigned int, unsigned long const*).
void PS4Context::_DeselectAllReadWriteTexturesImpl(unsigned int stages) {
    if (!_GraphicsResourcesActive()) {
        return;
    }

    for (unsigned int index = 0; index < kNumShaderStages; ++index) {
        if ((stages & (1U << index)) == 0) {
            continue;
        }
        _ClearGnmRwTextures(static_cast<RndShaderProgramType>(index));
    }
}

// Reconstructed from eboot.elf at 0x8E9940. The map has
// _DeselectAllSourceTexturesImpl(unsigned int, unsigned long const*).
void PS4Context::_DeselectAllSourceTexturesImpl(unsigned int stages) {
    if (!_GraphicsResourcesActive()) {
        return;
    }

    for (unsigned int index = 0; index < kNumShaderStages; ++index) {
        if ((stages & (1U << index)) == 0) {
            continue;
        }
        const auto stage = static_cast<RndShaderProgramType>(index);
        _ClearGnmTextures(stage);
        _ClearGnmBuffers(stage);
    }
}

// Reconstructed from eboot.elf at 0x8EA740. Copies the source's append
// counter from GDS into the destination's active storage.
void PS4Context::_CopyBufferCounter(
    const RndComputeBuffer& source,
    const RndComputeBuffer& dest) {
    const auto& ps4Source = static_cast<const PS4ComputeBuffer&>(source);
    const auto& ps4Dest = static_cast<const PS4ComputeBuffer&>(dest);
    _BindComputeRwBuffer(0, &ps4Source.ActiveBuffer());
    _CopyGdsToMemory(0, ps4Dest.ActiveStorage(), sizeof(std::uint32_t), true);
    _BindComputeRwBuffer(0, nullptr);
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
    case kShaderProgramVertex:
    case kShaderProgramPixel:
        _BindGraphicsSampler(type, slot, sampler);
        break;
    case kShaderProgramCompute:
        _BindComputeSampler(slot, sampler);
        break;
    case kShaderProgramHull:
    case kShaderProgramDomain:
    case kShaderProgramGeometry:
        break;
    }
}

// Reconstructed from eboot.elf at 0x8EA920.
void PS4Context::_DeactivateShaderProgramTypeImpl(RndShaderProgramType type) {
    switch (type) {
    case kShaderProgramVertex:
        _ClearVertexShader();
        break;
    case kShaderProgramGeometry:
        _ClearGeometryShader();
        break;
    case kShaderProgramPixel:
        _ClearPixelShader();
        break;
    case kShaderProgramCompute:
        _ClearComputeShader();
        break;
    case kShaderProgramHull:
    case kShaderProgramDomain:
        break;
    }
}

// Resource barriers ----------------------------------------------------------

void PS4Context::_SyncBarrierPhase(
    const RndResourceBarrier& barrier,
    std::uint32_t barrierCacheActions,
    volatile std::uint32_t*& sharedLabel,
    std::uint32_t& cacheActions,
    bool& needsCompletionWait) {
    switch (barrier.mPhase) {
    case RndResourceBarrierPhase::kImmediate:
        cacheActions |= barrierCacheActions;
        needsCompletionWait = true;
        break;
    case RndResourceBarrierPhase::kBegin:
        _SignalResource(barrier.mResource, sharedLabel);
        cacheActions |= barrierCacheActions;
        break;
    case RndResourceBarrierPhase::kEnd:
        _WaitForResource(barrier.mResource);
        break;
    }
}

void PS4Context::_ResolveTextureMetadata(
    const RndResourceBarrier& barrier,
    bool resolveDepth,
    bool& needsCompletionWait) {
    if (resolveDepth) {
        _ResolveDepthMetadata(barrier.mResource, barrier.mSubresource);
    } else {
        _ResolveColorMetadata(barrier.mResource, barrier.mSubresource);
    }

    volatile std::uint32_t* metadataLabel = nullptr;
    if (_RecordingCompute()) {
        const auto computeQueue = _ActiveComputeQueue();
        _SelectGraphics();
        _SignalResource(barrier.mResource, metadataLabel);
        _SelectCompute(computeQueue);

        if (barrier.mPhase != RndResourceBarrierPhase::kBegin) {
            _WaitForResource(barrier.mResource);
        }
    } else if (barrier.mPhase == RndResourceBarrierPhase::kBegin) {
        _SignalResource(barrier.mResource, metadataLabel);
    } else {
        needsCompletionWait = true;
    }
}

void PS4Context::_ProcessTransition(
    const RndResourceBarrier& barrier,
    volatile std::uint32_t*& sharedLabel,
    std::uint32_t& cacheActions,
    bool& needsCompletionWait) {
    if (barrier.mBefore == barrier.mAfter) {
        return;
    }

    if (IsState(barrier.mAfter, RndResourceState::kRenderTarget)) {
        _WaitForRenderTarget(barrier.mResource);
    }

    const auto barrierCacheActions = DestinationCacheActions(barrier.mAfter);

    if (IsState(barrier.mBefore, RndResourceState::kRenderTarget)) {
        if (barrier.mPhase == RndResourceBarrierPhase::kEnd) {
            _WaitForResource(barrier.mResource);
        } else {
            _ResolveTextureMetadata(barrier, false, needsCompletionWait);
        }
        return;
    }

    if (IsState(barrier.mBefore, RndResourceState::kDepthWrite)) {
        if (barrier.mPhase == RndResourceBarrierPhase::kEnd) {
            _WaitForResource(barrier.mResource);
        } else {
            _ResolveTextureMetadata(barrier, true, needsCompletionWait);
        }
        return;
    }

    if (IsState(barrier.mBefore, RndResourceState::kResolveDestination)) {
        return;
    }

    const bool sourceRequiresBarrier =
        IsState(barrier.mBefore, RndResourceState::kUnorderedAccess) ||
        IsState(barrier.mBefore, RndResourceState::kStreamOutput) ||
        IsState(barrier.mBefore, RndResourceState::kCopyDestination);
    if (sourceRequiresBarrier || IsWriteDestination(barrier.mAfter)) {
        _SyncBarrierPhase(
            barrier,
            barrierCacheActions,
            sharedLabel,
            cacheActions,
            needsCompletionWait);
    }
}

// Reconstructed from eboot.elf at 0x8EAA10.
void PS4Context::_ResourceBarrierImpl(unsigned long count, const RndResourceBarrier* barriers) {
    volatile std::uint32_t* sharedLabel = nullptr;
    std::uint32_t cacheActions = 0;
    bool needsCompletionWait = false;

    for (std::size_t index = 0; index < count; ++index) {
        const auto& barrier = barriers[index];
        switch (barrier.mType) {
        case RndResourceBarrierType::kTransition:
            _ProcessTransition(
                barrier, sharedLabel, cacheActions, needsCompletionWait);
            break;
        case RndResourceBarrierType::kAliasing:
            break;
        case RndResourceBarrierType::kUnorderedAccess:
            _SyncBarrierPhase(
                barrier,
                kInvalidateL1,
                sharedLabel,
                cacheActions,
                needsCompletionWait);
            break;
        }
    }

    if (needsCompletionWait) {
        auto completionCacheActions = cacheActions;
        if (_RecordingGraphics()) {
            completionCacheActions |= kWriteBackAndInvalidateL1L2;
            cacheActions = completionCacheActions;
        }
        _EmitTransitionCompletionWait(completionCacheActions);
    }

    if (cacheActions != 0) {
        _FlushTransitionCaches(cacheActions);
    }
}

// Reconstructed from eboot.elf at 0x8EB3E0.
void PS4Context::_SignalResource(
    const void* resource,
    volatile std::uint32_t*& sharedLabel) {
    if (sharedLabel == nullptr) {
        sharedLabel = _AllocateResourceLabel();
        *sharedLabel = 0;

        if (_RecordingGraphics()) {
            _EmitGraphicsResourceSignal(sharedLabel, kResourceReadyValue);
        } else if (_RecordingCompute()) {
            _EmitComputeResourceSignal(sharedLabel, kResourceReadyValue);
        }
    }

    const ResourceSignal signal = {
        resource,
        sharedLabel,
        TheRndDevice()->mFrameCount,
    };
    _TrackResourceSignal(signal);
}

// Reconstructed from eboot.elf at 0x8EB590.
void PS4Context::_WaitForResource(const void* resource) {
    auto* signal = _FindResourceSignal(resource);
    if (signal == nullptr) {
        return;
    }

    if (_RecordingGraphics()) {
        _EmitGraphicsResourceWait(signal->mLabel, kResourceReadyValue);
    } else if (_RecordingCompute()) {
        _EmitComputeResourceWait(signal->mLabel, kResourceReadyValue);
    }

    _RemoveResourceSignalGroup(signal->mLabel);
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
    _BindDepthClearShader();
    _SetDepthClearDrawState(false);
    _UnbindPixelShader();
    _SubmitDepthClearDraw();
    _SetDepthClearDrawState(true);
}
