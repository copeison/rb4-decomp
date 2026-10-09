#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <gnm/buffer.h>
#include <gnm/depthrendertarget.h>
#include <gnm/rendertarget.h>
#include <gnm/sampler.h>
#include <gnm/texture.h>
#include <gnmx/computecontext.h>
#include <gnmx/computequeue.h>
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

    PS4Context();            // 0x8E72B0
    ~PS4Context() override;  // 0x8E8070, 0x8E82B0

    void _SignalFenceImpl(RndFence& fence) override;            // 0x8EB730
    void _WaitFenceImpl(const RndFence& fence) override;        // 0x8EB7F0
    void _BeginFrameImpl(unsigned int flags) override;          // 0x8E8850
    void _SetRenderTargetsImpl(
        RndTargetMode mode,
        const RenderTargetParams& params) override;           // 0x8E8D20
    void _SetBlendModeImpl(
        RndBlendMode mode,
        const Hmx::Color& blendColor) override;               // 0x8E92D0
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
    // Clears a depth target: with HTILE, by clearing the HTILE and any
    // separate stencil with the clear compute shader; otherwise with the
    // fast-clear draw. The map has _ClearDepthStencil(float, unsigned char);
    // this build also passes the depth target and returns whether it
    // dispatched compute work.
    bool _ClearDepthStencil(
        const sce::Gnm::DepthRenderTarget& target,
        float depth,
        unsigned char stencil);  // 0x8E99E0
    // Draws a full-target quad with color writes off, so the depth block
    // applies its clear.
    void _FlushClear();          // 0x8EBDA0

private:
    // Allocates each frame's graphics-context buffers and initializes the
    // context with its CUE ring setup, global resource table and GS rings.
    void _CreateGfxContext();  // 0x8E7AF0
    void _CreateGpuTimestampPool();  // 0x8E7DF0

    // Forgets SetupDraw's state, turns the GS mode off and enables color
    // writes. Inlined into _BeginFrameImpl and _ResetFrame; name not in the
    // reference map.
    void _ResetDrawState();
    // Rebuild and set the Gnm depth-stencil and primitive-setup state from
    // the cache. Map names; inlined into the setters in this build.
    void _SyncDepthStencilControl();
    void _SyncPrimitiveSetup();

    // Whether commands go to the graphics or a compute context. Names not
    // in the reference map.
    bool _RecordingGraphics() const;
    bool _RecordingCompute() const;
    sce::Gnm::EndOfPipeEventType _GpuTimestampEvent() const;

    // Writes a label at the end of the pipe with the given cache actions
    // and waits for it, idling the graphics ring. Inlined into
    // _SetRenderTargetsImpl and _ResourceBarrierImpl; name not in the
    // reference map.
    void _WaitForEndOfPipe(std::uint32_t cacheActions);
    // Leaving render-target or depth-write state: waits for the target's
    // writes, then decompresses it. Inlined into _ResourceBarrierImpl; names
    // not in the reference map.
    void _DecompressColorTarget(const RndResourceBarrier& barrier, std::uint32_t cacheActions);
    void _DecompressDepthTarget(const RndResourceBarrier& barrier, std::uint32_t cacheActions);

    // Split-barrier resource signals. Names not in the reference map.
    void _SignalResource(
        const void* resource,
        volatile std::uint32_t*& sharedLabel);  // 0x8EB3E0
    void _WaitForResource(const void* resource);  // 0x8EB590
    // Hands out the next label of the ring, growing it when it would reach
    // the labels the previous frame may still use. Name not in the
    // reference map.
    volatile std::uint32_t* _AllocateResourceLabel();  // 0x8E7EB0

public:
    // Layout is modeled only where the offsets are known. Field names are
    // not in the reference map. The first range starts in RndContext's tail
    // padding at 0x5721.
    unsigned char mUnknown22305[7];
    // One Gnmx graphics context per frame slot.
    sce::Gnmx::GfxContext mGfxContexts[kFrameSlotCount];
    // Per frame slot: the CUE heap, the draw command buffer and the constant
    // command buffer. Then the global resource table and the ES-GS and GS-VS
    // rings, which each slot reallocates and the last one keeps.
    void* mCueHeaps[kFrameSlotCount];
    void* mDrawCommandBuffers[kFrameSlotCount];
    void* mConstantCommandBuffers[kFrameSlotCount];
    void* mGlobalResourceTable;
    void* mEsGsRing;
    void* mGsVsRing;
    // Nonzero while a frame's graphics (0) or compute (1-9) submission is
    // in flight.
    volatile std::int32_t mSubmissionPending[kFrameSlotCount][10];
    // Resources signalled by split barriers, and the label ring their
    // signals use: the labels, the first label of each frame parity, the
    // frame the ring was last used in, the next label and the capacity.
    // Names not in the reference map.
    std::vector<ResourceSignal> mResourceSignals;
    std::uint32_t* mLabels;
    unsigned long mLabelFrameStarts[2];
    unsigned long mLabelFrame;
    unsigned long mNextLabel;
    unsigned long mLabelCapacity;
    // The medium-priority queue on pipe 1, for the first three compute
    // contexts of a frame, and the low-priority queue on pipe 0.
    sce::Gnmx::ComputeQueue mComputeQueues[2];
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
static_assert(offsetof(PS4Context, mCueHeaps) == 0x22838);
static_assert(offsetof(PS4Context, mGlobalResourceTable) == 0x22868);
static_assert(offsetof(PS4Context, mGsVsRing) == 0x22878);
static_assert(offsetof(PS4Context, mSubmissionPending) == 0x22880);
static_assert(sizeof(std::vector<PS4Context::ResourceSignal>) == 32);
static_assert(offsetof(PS4Context, mResourceSignals) == 0x228D0);
static_assert(offsetof(PS4Context, mLabels) == 0x228F0);
static_assert(offsetof(PS4Context, mLabelCapacity) == 0x22918);
static_assert(sizeof(sce::Gnmx::ComputeQueue) == 88);
static_assert(offsetof(PS4Context, mComputeQueues) == 0x22920);
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
