#pragma once

#include <cstddef>
#include <cstdint>

#include "render/shaders/RndShaderEnums.h"
#include "utl/containers/Vector.h"

class RndComputeBuffer;
class RndFence;
class RndShaderCBuffer;

namespace rb4 {
enum class RndMaterialBlendMode : std::int32_t;
}  // namespace rb4

struct RndResourceBarrier;

enum RndVertexType : unsigned int;

enum RndCullMode : unsigned int {
    kCullNone = 0,
    kCullBack = 1,
    kCullFront = 2,
};

enum RndWriteMaskChannelSet : unsigned int {
    kWriteRGBA = 0,
    kWriteRGB = 1,
    kWriteNone = 2,
};

// Primitive types for transient draws. Enumerator names are not in the
// reference map.
enum class RndPrimitive : unsigned int {
    kTriangles = 3,
};

// Camera and projection state, built by the constructor at 0x3D8170 and
// embedded twice in the context. The name is inferred from the map's
// RndCameraContext; its layout has not been recovered.
class RndCameraContext {
public:
    RndCameraContext();

    unsigned char mUnknown[9312];
};

static_assert(sizeof(RndCameraContext) == 9312);

// Fixed-capacity vector with inline storage. Name from the map's
// FixedVector<T, N>.
template <typename T, unsigned long N>
struct FixedVector {
    T* mData;
    unsigned long mSize;
    unsigned long mCapacity;
    T mStorage[N];
};

// GPU timing results for one statistic. Name not in the reference map.
struct RndGpuStatSample {
    float mSeconds;
    unsigned int mUnknown4;
    unsigned long mCounters[6];
};

static_assert(sizeof(RndGpuStatSample) == 56);

// Sixteen-byte record reserved 2000 at a time by the constructor. Name not in
// the reference map.
struct RndContextRecord16 {
    unsigned char mBytes[16];
};

// Open GPU statistic scope. Name not in the reference map.
struct RndGpuStatScope {
    void* mStat;
    unsigned long mKey;
};

// Draw state shared by every platform context. The base vtable is at
// 0x19376C0; the PS4 context derives from it.
class RndContext {
public:
    // Render-target binding. Defined by the PS4 context code until the
    // render-target classes are converted. Name from the map.
    struct RenderTargetParams;
    // Per-target blend configuration. Name not in the reference map.
    struct BlendParams;

    explicit RndContext(bool disableComputeQueues);  // 0x6BBDE0
    virtual ~RndContext();                           // 0x6BC120, 0x6BC190

    virtual void Init();       // slot 2 at 0x6BC200
    virtual void Terminate();  // slot 3 at 0x6BC330
    // Slots 4-5 are newer than the reference map.
    virtual void _SignalFenceImpl(RndFence& fence);       // 0x6BDA50
    virtual void _WaitFenceImpl(const RndFence& fence);   // 0x6BDA60
    virtual void _FinishImpl();                           // 0x6BDA70
    virtual void _BeginFrameImpl();                       // 0x6BDA80
    virtual void _SetRenderTargetsImpl(int mode, const RenderTargetParams& params) = 0;
    virtual void _SetBlendModeImpl(
        rb4::RndMaterialBlendMode mode,
        const BlendParams& params) = 0;
    // The map types the parameter as RndDepthMode.
    virtual void _SetDepthModeImpl(unsigned int mode) = 0;
    // The map has _SetStencilModeImpl(RndStencilMode, unsigned char); this
    // build adds the read and write mask indices.
    virtual void _SetStencilModeImpl(
        unsigned int mode,
        unsigned char reference,
        unsigned int readMask,
        unsigned int writeMask) = 0;
    // The map types the parameter as RndFrontFace.
    virtual void _SetFrontFaceImpl(bool counterClockwise) = 0;
    virtual void _SetCullModeImpl(RndCullMode mode) = 0;
    // The map types the parameter as RndFillMode.
    virtual void _SetFillModeImpl(bool solid) = 0;
    virtual void _SetColorWriteMaskImpl(
        unsigned char targets,
        RndWriteMaskChannelSet channels) = 0;
    virtual void _SetDepthClipEnabledImpl(bool enabled) = 0;
    virtual void _SetDepthBiasEnabledImpl(bool enabled) = 0;
    virtual void _SetThickLinesImpl(bool enabled) = 0;
    virtual void _DispatchComputeImpl(unsigned int x, unsigned int y, unsigned int z) = 0;
    // The map types the parameter as RndPipeline.
    virtual void _SetActivePipelineImpl(int pipeline);  // 0x6BDA90
    // Slot 21 at 0x6BDAA0. Name not in the reference map.
    virtual void _ResourceBarrierImpl(
        unsigned long count,
        const RndResourceBarrier* barriers);
    virtual void _DrawPrimitivesImpl(
        RndPrimitive primitive,
        RndVertexType type,
        const void* vertices,
        unsigned long count) = 0;
    virtual void _DrawIndirectImpl(RndPrimitive primitive, const RndComputeBuffer& args) = 0;
    virtual void _CopyBufferCounter(
        const RndComputeBuffer& source,
        const RndComputeBuffer& dest) = 0;
    // The map has (unsigned int, unsigned long const*); this build passes a
    // stage mask only.
    virtual void _DeselectAllReadWriteTexturesImpl(unsigned int stages) = 0;
    virtual void _DeselectAllSourceTexturesImpl(unsigned int stages) = 0;
    // Slot 27. Name not in the reference map.
    virtual void _SetSamplerImpl(
        RndShaderProgramType type,
        unsigned int slot,
        unsigned int wrap,
        unsigned int filter) = 0;
    virtual void _DeactivateShaderProgramTypeImpl(RndShaderProgramType type) = 0;
    virtual void _PushMarkerImpl(const char* name);             // 0x6BDAB0
    virtual void _PopMarkerImpl();                              // 0x6BDAC0
    virtual void _BeginGpuStatsImpl(unsigned long key);         // 0x6BDAD0
    virtual void _EndGpuStatsImpl(unsigned long key);           // 0x6BDAE0
    virtual RndGpuStatSample _EvalAndRetireGpuStatsImpl(unsigned long key);  // 0x6BDAF0

    // Reconstructed from eboot.elf at 0x6BD420.
    void DeactivateShaderProgramType(RndShaderProgramType type);
    // Starts a frame with the given activation flags. Not yet reconstructed;
    // name not in the reference map.
    void BeginFrame(unsigned int flags);

    // Statistic scopes opened on this context. Names not in the reference
    // map.
    RndGpuStatScope* LastGpuStatScope();
    void PushGpuStatScope(const RndGpuStatScope& scope);
    void PopGpuStatScope();

    // Field names are not in the reference map.
    bool mFrameActive;
    bool mDisableComputeQueues;
    int mMode;
    int mSliceMode;  // -1 selects a single render-target slice.
    FixedVector<void*, 8> mUnknown24;
    unsigned char mUnknown112[28];
    float mUnknown140;
    RndCameraContext mCameras[2];
    bool mUnknown18768;
    unsigned long mUnknown18776;
    unsigned char mActiveShaderStages;  // A bit per stage with a program.
    int mUnknown18788;
    unsigned long mInputSlotLimits[kNumShaderProgramTypes];
    unsigned long mOutputSlotLimits[kNumShaderProgramTypes];
    bool mUnknown18888;
    int mUnknown18892;
    int mUnknown18896;
    float mUnknown18900;
    int mUnknown18904;
    unsigned char mUnknown18908[60];
    int mShadingMode;
    int mUnknown18972;
    int mUnknown18976;
    unsigned char mUnknown18980[12];
    eastl::vector<RndContextRecord16> mUnknown18992;
    // Global constant buffers created by Init; the last three are the
    // per-draw buffers of 16, 32, and 64 elements.
    RndShaderCBuffer* mCBuffers[9];
    unsigned char mUnknown19096[3168];
    unsigned char mUnknown22264[8];
    eastl::vector<RndGpuStatScope> mGpuStatScopes;
    bool mUnknown22304;
};

static_assert(offsetof(RndContext, mSliceMode) == 0x10);
static_assert(offsetof(RndContext, mUnknown24) == 24);
static_assert(offsetof(RndContext, mCameras) == 144);
static_assert(offsetof(RndContext, mUnknown18768) == 18768);
static_assert(offsetof(RndContext, mActiveShaderStages) == 0x4960);
static_assert(offsetof(RndContext, mUnknown18788) == 0x4964);
static_assert(offsetof(RndContext, mInputSlotLimits) == 0x4968);
static_assert(offsetof(RndContext, mOutputSlotLimits) == 0x4998);
static_assert(offsetof(RndContext, mUnknown18888) == 18888);
static_assert(offsetof(RndContext, mShadingMode) == 0x4A18);
static_assert(offsetof(RndContext, mUnknown18980) == 0x4A24);
static_assert(offsetof(RndContext, mUnknown18992) == 18992);
static_assert(offsetof(RndContext, mCBuffers) == 0x4A50);
static_assert(offsetof(RndContext, mUnknown19096) == 19096);
static_assert(offsetof(RndContext, mUnknown22264) == 0x56F8);
static_assert(offsetof(RndContext, mGpuStatScopes) == 0x5700);
static_assert(offsetof(RndContext, mUnknown22304) == 22304);
static_assert(sizeof(RndContext) == 0x5728);
