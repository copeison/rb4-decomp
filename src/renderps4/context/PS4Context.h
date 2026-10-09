#pragma once

#include <cstddef>
#include <cstdint>
#include <gnm/buffer.h>
#include <gnm/depthrendertarget.h>
#include <gnm/rendertarget.h>
#include <gnm/sampler.h>
#include <gnm/texture.h>
#include <gnmx/computecontext.h>
#include <gnmx/gfxcontext.h>

#include "render/context/RndContext.h"
#include "renderps4/buffers/PS4TransientBuffer.h"
#include "renderps4/context/PS4RenderStateUtl.h"
#include "utl/containers/Map.h"

class PS4Device;

// PS4 context: two graphics contexts, compute queues, transient vertex
// buffers, and submission state. The vtable is at 0x195FDB0.
class PS4Context : public RndContext {
public:
    // Frame slots, compute contexts and transient vertex formats. Names not
    // in the reference map.
    static constexpr std::size_t kFrameSlotCount = 2;
    static constexpr std::size_t kComputeContextCount = 18;
    static constexpr std::size_t kComputeContextsPerFrame = 9;
    static constexpr std::size_t kTransientFormatCount = 8;

    // GPU timestamp pair for one statistic key; the map keys an
    // eastl::map<unsigned long, GpuStatBlock*> by it. Field names are not in
    // the reference map.
    struct GpuStatBlock {
        bool mActive;
        volatile std::uint64_t* mBegin;
        volatile std::uint64_t* mEnd;
    };
    // GPU-stat blocks in the ring. Name not in the reference map.
    static constexpr std::size_t kNumGpuStatBlocks = 512;

    // A resource signalled by a split barrier, with the frame it was
    // signalled in. Name not in the reference map.
    struct ResourceSignal {
        const void* mResource = nullptr;
        volatile std::uint32_t* mLabel = nullptr;
        std::uint64_t mRenderEpoch = 0;
    };

    // Viewport of a render-target binding. Name not in the reference map.
    struct ViewportRect {
        float mX;
        float mY;
        float mWidth;
        float mHeight;
    };

    // GPU range a depth or HTILE clear fills. Name not in the reference map.
    struct DepthClearRange {
        std::uint64_t mGpuAddress = 0;
        std::uint32_t mDwordCount = 0;
    };

    PS4Context();            // 0x8E72B0
    ~PS4Context() override;  // 0x8E8070, 0x8E82B0

    void _SignalFenceImpl(RndFence& fence) override;            // 0x8EB730
    void _WaitFenceImpl(const RndFence& fence) override;        // 0x8EB7F0
    void _BeginFrameImpl() override;                            // 0x8E8850
    void _SetRenderTargetsImpl(int mode, const RenderTargetParams& params) override;  // 0x8E8D20
    void _SetBlendModeImpl(
        RndBlendMode mode,
        const BlendParams& params) override;                    // 0x8E92D0
    void _SetDepthModeImpl(unsigned int mode) override;         // 0x8E9F60
    void _SetStencilModeImpl(
        unsigned int mode,
        unsigned char reference,
        unsigned int readMask,
        unsigned int writeMask) override;                       // 0x8EA030
    void _SetFrontFaceImpl(bool counterClockwise) override;     // 0x8EA150
    void _SetCullModeImpl(RndCullMode mode) override;           // 0x8EA1C0
    void _SetFillModeImpl(bool solid) override;                 // 0x8EA230
    void _SetColorWriteMaskImpl(
        unsigned char targets,
        RndWriteMaskChannelSet channels) override;              // 0x8E96F0
    void _SetDepthClipEnabledImpl(bool enabled) override;       // 0x8EA2A0
    void _SetDepthBiasEnabledImpl(bool enabled) override;       // 0x8EA2B0
    void _SetThickLinesImpl(bool enabled) override;             // 0x8EA2C0
    void _DispatchComputeImpl(unsigned int x, unsigned int y, unsigned int z) override;  // 0x8EB870
    void _ResourceBarrierImpl(
        unsigned long count,
        const RndResourceBarrier* barriers) override;   // 0x8EAA10
    void _DrawPrimitivesImpl(
        RndPrimitive primitive,
        RndVertexType type,
        const void* vertices,
        unsigned long count) override;                          // 0x8EA2D0
    void _DrawIndirectImpl(RndPrimitive primitive, const RndComputeBuffer& args) override;  // 0x8EA730
    void _CopyBufferCounter(
        const RndComputeBuffer& source,
        const RndComputeBuffer& dest) override;                 // 0x8EA740
    void _DeselectAllReadWriteTexturesImpl(unsigned int stages) override;  // 0x8E9810
    void _DeselectAllSourceTexturesImpl(unsigned int stages) override;     // 0x8E9940
    void _SetSamplerImpl(
        RndShaderProgramType type,
        unsigned int slot,
        unsigned int wrap,
        unsigned int filter) override;                          // 0x8EA830
    void _DeactivateShaderProgramTypeImpl(RndShaderProgramType type) override;  // 0x8EA920
    void _PushMarkerImpl(const char* name) override;            // 0x8EB970
    void _PopMarkerImpl() override;                             // 0x8EB9D0
    void _BeginGpuStatsImpl(unsigned long key) override;        // 0x8EBA20
    void _EndGpuStatsImpl(unsigned long key) override;          // 0x8EBBB0
    RndGpuStatSample _EvalAndRetireGpuStatsImpl(unsigned long key) override;  // 0x8EBC70

    // Creates the immediate context and installs it on the device. Name not
    // in the reference map.
    static PS4Context* _CreateImmediate(PS4Device& device);

    // Activates the shader stages the bound programs need, turns the GS mode
    // off when no geometry shader is bound, and sets the primitive type,
    // skipping state the context already holds. The map has
    // SetupDraw(RndContext::Primitive); this build's enum is RndPrimitive.
    void SetupDraw(RndPrimitive primitive);  // 0x8EA560
    // Enables or disables color-buffer writes, skipping the packet when the
    // state is unchanged.
    void SetCbEnabled(bool enabled);  // 0x8EA7D0
    // The compute context of the active frame slot and compute slot. Name
    // not in the reference map.
    sce::Gnmx::ComputeContext& _ActiveComputeContext();
    // The active frame's graphics context. Name not in the reference map.
    sce::Gnmx::GfxContext& _ActiveGfxContext();

    void SubmitFrame();  // 0x8E82D0
    // Resets the active frame's command state before recording. Name not in
    // the reference map; it may be the map's _ResetImpl().
    void _ResetFrame();  // 0x8E8450
    // Whether every submission of the active or the given frame has
    // retired. Names not in the reference map.
    bool _SubmissionsComplete() const;
    bool _FrameSubmissionsComplete(std::size_t frame) const;
    // The map has _ClearDepthStencil(float, unsigned char); this build also
    // passes the depth target and returns whether HTILE was cleared.
    bool _ClearDepthStencil(
        const sce::Gnm::DepthRenderTarget& target,
        float depth,
        unsigned char stencil);  // 0x8E99E0
    void _FlushClear();          // 0x8EBDA0

private:
    // Construction and teardown. Names not in the reference map; they are
    // not yet reconstructed unless an address is given. The graphics
    // and compute contexts are members, so the compiler constructs and
    // destroys them as the binary does; _InitCommandState and
    // _DestructCommandState stand for the rest of the command state.
    void _InitCommandState();
    void _InitStateDefaults();
    void _CreateGfxContext();  // 0x8E7AF0
    void _InitGfxSlot(
        std::size_t slot,
        std::size_t cueHeapSize,
        std::size_t drawCommandBufferSize,
        std::size_t resourceBufferSize,
        std::size_t constantUpdateSize,
        std::size_t scratchBufferSize);
    void _CreateGpuTimestampPool();  // 0x8E7DF0
    void _InitComputeQueue(
        std::size_t queue,
        std::uint32_t pipe,
        std::uint32_t priority,
        std::size_t ringSize,
        std::size_t ringAlignment);
    void _InitComputeContext(
        std::size_t slot,
        std::size_t cueSlotCount,
        std::size_t commandBufferSize);
    void _InitLabelPool(std::size_t initialCapacity);
    void _ReleaseLabelPool();
    void _DestructCommandState();

    // Frame submission and reset. Names not in the reference map; not yet
    // reconstructed.
    void _EmitEndOfFrameEvent(std::size_t frame);
    void _EmitComputeCompletion(std::size_t frame, std::size_t slot);
    void _SubmitCompute(std::size_t frame, std::size_t slot, std::size_t queue);
    void _EmitGfxCompletion(std::size_t frame);
    void _SubmitGfx(std::size_t frame);
    void _ResetGfxSlot(std::size_t frame);
    void _InitGfxHardwareState(std::size_t frame);
    void _ClearFrameDrawCount(std::size_t frame);
    void _ResetComputeSlot(std::size_t frame, std::size_t slot);
    void _InitFrameCommandState(std::size_t frame);
    void _EmitDefaultControlState(std::size_t frame);

    // Pipeline state. _SyncDepthStencilControl and _SyncPrimitiveSetup are
    // map names; the others are not in the reference map. Not yet
    // reconstructed.
    void _BindColorTarget(std::size_t slot, const sce::Gnm::RenderTarget* target);
    void _BindDepthTarget(const sce::Gnm::DepthRenderTarget* target);
    void _SetViewportAndScissor(const ViewportRect& viewport);
    void _BeginRenderTargetSync();
    void _PrepareColorTarget(const RenderTargetParams& params, std::size_t slot);
    bool _PrepareDepthTarget(
        const sce::Gnm::DepthRenderTarget& target,
        const RenderTargetParams& params);
    void _FinishRenderTargetSync();
    void _ResetCachedPipelineState();
    void _SetDefaultRasterState();
    void _SetDefaultDepthStencilState();
    void _DisableStreamOutput();
    void _ClearShaderResources();
    // Rebuild and set the Gnm depth-stencil and primitive-setup state from
    // the cache. Map names; inlined into the setters in this build.
    void _SyncDepthStencilControl();
    void _SyncPrimitiveSetup();

    // Whether commands go to the graphics or a compute context. Names not
    // in the reference map.
    bool _RecordingGraphics() const;
    bool _RecordingCompute() const;
    sce::Gnm::EndOfPipeEventType _GpuTimestampEvent() const;

    // Depth clears. Names not in the reference map; not yet reconstructed.
    void _FlushDepthMetadata();
    void _DispatchDepthClear(const DepthClearRange& range, std::uint32_t clearValue);
    void _BeginRasterDepthClear(float depth, std::uint8_t stencil);
    void _FinishRasterDepthClear();
    void _BindDepthClearShader();
    void _SetDepthClearDrawState(bool enabled);
    void _UnbindPixelShader();
    void _SubmitDepthClearDraw();

    // Resource barriers. Names not in the reference map; not yet
    // reconstructed unless an address is given.
    void _SyncBarrierPhase(
        const RndResourceBarrier& barrier,
        std::uint32_t barrierCacheActions,
        volatile std::uint32_t*& sharedLabel,
        std::uint32_t& cacheActions,
        bool& needsCompletionWait);
    void _ResolveTextureMetadata(
        const RndResourceBarrier& barrier,
        bool resolveDepth,
        bool& needsCompletionWait);
    void _ProcessTransition(
        const RndResourceBarrier& barrier,
        volatile std::uint32_t*& sharedLabel,
        std::uint32_t& cacheActions,
        bool& needsCompletionWait);
    std::uint64_t _ActiveComputeQueue() const;
    void _SelectGraphics();
    void _SelectCompute(std::uint64_t queueIndex);
    void _WaitForRenderTarget(const void* resource);
    void _ResolveColorMetadata(const void* resource, std::uint64_t subresource);
    void _ResolveDepthMetadata(const void* resource, std::uint64_t subresource);
    void _EmitTransitionCompletionWait(std::uint32_t cacheActions);
    void _FlushTransitionCaches(std::uint32_t cacheActions);

    // Split-barrier resource signals. Names not in the reference map; not
    // yet reconstructed unless an address is given.
    void _SignalResource(
        const void* resource,
        volatile std::uint32_t*& sharedLabel);  // 0x8EB3E0
    void _WaitForResource(const void* resource);  // 0x8EB590
    volatile std::uint32_t* _AllocateResourceLabel();
    void _EmitGraphicsResourceSignal(volatile std::uint32_t* label, std::uint32_t value);
    void _EmitComputeResourceSignal(volatile std::uint32_t* label, std::uint32_t value);
    void _TrackResourceSignal(const ResourceSignal& signal);
    ResourceSignal* _FindResourceSignal(const void* resource);
    void _EmitGraphicsResourceWait(const volatile std::uint32_t* label, std::uint32_t value);
    void _EmitComputeResourceWait(const volatile std::uint32_t* label, std::uint32_t value);
    void _RemoveResourceSignalGroup(const volatile std::uint32_t* label);

    // Shader stages, samplers and resource tables. Names not in the
    // reference map; not yet reconstructed.
    void _BindComputeRwBuffer(std::uint32_t slot, const sce::Gnm::Buffer* buffer);
    void _CopyGdsToMemory(
        std::uint32_t gdsOffset,
        void* destination,
        std::size_t size,
        bool blocking);
    void _BindGraphicsSampler(
        RndShaderProgramType stage,
        std::uint32_t slot,
        const sce::Gnm::Sampler& sampler);
    void _BindComputeSampler(std::uint32_t slot, const sce::Gnm::Sampler& sampler);
    void _ClearVertexShader();
    void _ClearGeometryShader();
    void _ClearPixelShader();
    void _ClearComputeShader();
    bool _GraphicsResourcesActive() const;
    void _ClearGnmRwTextures(RndShaderProgramType stage);
    void _ClearGnmTextures(RndShaderProgramType stage);
    void _ClearGnmBuffers(RndShaderProgramType stage);

public:
    // Layout is modeled only where the offsets are known. Field names are
    // not in the reference map. The first range starts in RndContext's tail
    // padding at 0x5721.
    unsigned char mUnknown22305[7];
    // One Gnmx graphics context per frame slot.
    sce::Gnmx::GfxContext mGfxContexts[kFrameSlotCount];
    unsigned char mUnknown141368[0x48];
    // Nonzero while a frame's graphics (0) or compute (1-9) submission is
    // in flight.
    volatile std::int32_t mSubmissionPending[kFrameSlotCount][10];
    unsigned char mUnknown141520[0x100];
    // Nine compute contexts per frame slot.
    sce::Gnmx::ComputeContext mComputeContexts[kFrameSlotCount][kComputeContextsPerFrame];
    std::size_t mActiveFrame;
    // The cached depth, stencil, raster and color-write state the setters
    // rebuild the Gnm controls from. Names not in the reference map.
    unsigned int mDepthMode;
    unsigned int mStencilMode;
    unsigned int mFrontFace;  // 1 is counter-clockwise.
    RndCullMode mCullMode;
    unsigned int mFillMode;   // 1 is solid.
    std::uint8_t mStencilReference;
    std::uint8_t mStencilReadMask;
    std::uint8_t mStencilWriteMask;
    unsigned int mColorWriteTargets;
    RndWriteMaskChannelSet mColorWriteChannels;
    // One bank per frame, indexed by vertex type.
    PS4TransientBuffer mTransientBuffers[kFrameSlotCount][kTransientFormatCount];
    // GPU-stat timestamp pairs, used as a ring, with their GPU memory and
    // the next block to hand out, and the blocks of the open statistics by
    // key. Names not in the reference map.
    GpuStatBlock mGpuStatBlocks[kNumGpuStatBlocks];
    std::uint64_t* mGpuTimestamps;
    unsigned long mNextGpuStatBlock;
    eastl::map<unsigned long, GpuStatBlock*> mGpuStats;
    // SetupDraw's record of the state it last set on the graphics context:
    // the active shader stages, the primitive type, and whether the GS mode
    // may be on. Names not in the reference map.
    sce::Gnm::ActiveShaderStages mCachedShaderStages;
    sce::Gnm::PrimitiveType mCachedPrimitiveType;
    bool mGsModeEnabled;
    // SetCbEnabled's last state. Name not in the reference map.
    bool mCbEnabled;
    unsigned char mUnknown280714[6];
};

static_assert(sizeof(sce::Gnmx::GfxContext) == 0xE888);
static_assert(sizeof(PS4Context::GpuStatBlock) == 24);
static_assert(sizeof(PS4Context::ResourceSignal) == 24);
static_assert(offsetof(PS4Context, mUnknown22305) == 0x5721);
static_assert(offsetof(PS4Context, mGfxContexts) == 0x5728);
static_assert(offsetof(PS4Context, mUnknown141368) == 0x22838);
static_assert(offsetof(PS4Context, mSubmissionPending) == 0x22880);
static_assert(offsetof(PS4Context, mUnknown141520) == 0x228D0);
static_assert(sizeof(sce::Gnmx::ComputeContext) == 0x1AE0);
static_assert(offsetof(PS4Context, mComputeContexts) == 0x229D0);
static_assert(offsetof(PS4Context, mActiveFrame) == 0x40D90);
static_assert(offsetof(PS4Context, mDepthMode) == 0x40D98);
static_assert(offsetof(PS4Context, mStencilReference) == 0x40DAC);
static_assert(offsetof(PS4Context, mColorWriteTargets) == 0x40DB0);
static_assert(offsetof(PS4Context, mTransientBuffers) == 0x40DB8);
static_assert(offsetof(PS4Context, mGpuStatBlocks) == 0x41838);
static_assert(offsetof(PS4Context, mGpuTimestamps) == 0x44838);
static_assert(offsetof(PS4Context, mNextGpuStatBlock) == 0x44840);
static_assert(offsetof(PS4Context, mGpuStats) == 0x44848);
static_assert(sizeof(eastl::map<unsigned long, PS4Context::GpuStatBlock*>) == 56);
static_assert(offsetof(PS4Context, mCachedShaderStages) == 0x44880);
static_assert(offsetof(PS4Context, mCachedPrimitiveType) == 0x44884);
static_assert(offsetof(PS4Context, mGsModeEnabled) == 0x44888);
static_assert(offsetof(PS4Context, mCbEnabled) == 0x44889);
static_assert(offsetof(PS4Context, mUnknown280714) == 0x4488A);
static_assert(sizeof(PS4Context) == 0x44890);
