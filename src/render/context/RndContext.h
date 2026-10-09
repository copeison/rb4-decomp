#pragma once

#include <cstddef>
#include <cstdint>

#include "math/color/Color.h"
#include "render/shaders/RndShaderEnums.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"

class GameObject;
class RndComputeBuffer;
class RndFence;
class RndShaderCBuffer;
class RndTextureBase;

enum class RndBlendMode : std::int32_t;

struct RndResourceBarrier;

enum RndVertexType : unsigned int;

// What kind of texture a render-target binding draws into. The name is the
// map's; the enumerators are not in the reference map.
enum RndTargetMode : int {
    kTargetModeNone = -1,
    kTargetMode2D = 0,    // 2D textures and slices of 2D arrays.
    kTargetModeCube = 2,  // Cube textures.
};

// Where a context records: the graphics ring or a compute queue. The name
// is the map's; the enumerators are not in the reference map.
enum RndPipeline : int {
    kPipelineGraphics = 0,
    kPipelineCompute = 1,
};

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
    kTriangleStrip = 4,
};

// Camera and projection state, built by the constructor at 0x3D8170 and
// embedded twice in the context. The name is inferred from the map's
// RndCameraContext; its layout has not been recovered.
class RndCameraContext {
public:
    RndCameraContext();

    // Forgets the camera and its target. Not reconstructed yet.
    void Clear();  // 0x3D88C0
    // Writes the constants of a context without a camera for the target
    // mode. Not reconstructed yet.
    static void SetDefaultShaderConstants(RndTargetMode mode, RndShaderCBuffer& cbuffer);  // 0x3D9BC0

    unsigned char mUnknown[9312];
};

static_assert(sizeof(RndCameraContext) == 9312);

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
    // Render-target binding: up to eight color targets, each optionally
    // cleared, and a depth target, with the clear values and the viewport.
    // Name from the map; field names are not in the reference map.
    struct RenderTargetParams {
        struct Target {
            RndTextureBase* mTexture;
            int mClearMode;        // 1 clears the target.
            unsigned long mSlice;  // Array slice, or -1.
        };

        RenderTargetParams()
            : mClearColor(Hmx::Color::GetZero()),
              mDepthClear(0.0F),
              mStencilClear(0),
              mViewportX(0.0F),
              mViewportY(0.0F),
              mViewportWidth(0.0F),
              mViewportHeight(0.0F),
              mMinDepth(0.0F),
              mMaxDepth(1.0F),
              mDepthTexture(nullptr),
              mDepthClearMode(0),
              mDepthSlice(static_cast<unsigned long>(-1)) {}

        Hmx::Color mClearColor;
        float mDepthClear;
        unsigned char mStencilClear;
        float mViewportX;
        float mViewportY;
        float mViewportWidth;
        float mViewportHeight;
        float mMinDepth;
        float mMaxDepth;
        FixedVector<Target, 8> mTargets;
        RndTextureBase* mDepthTexture;
        int mDepthClearMode;  // 1 clears depth and stencil.
        unsigned long mDepthSlice;
    };

    explicit RndContext(bool disableComputeQueues);  // 0x6BBDE0
    virtual ~RndContext();                           // 0x6BC120, 0x6BC190

    virtual void Init();       // slot 2 at 0x6BC200
    virtual void Terminate();  // slot 3 at 0x6BC330
    // Slots 4-5 are newer than the reference map.
    virtual void _SignalFenceImpl(RndFence& fence);       // 0x6BDA50
    virtual void _WaitFenceImpl(const RndFence& fence);   // 0x6BDA60
    virtual void _FinishImpl();                           // 0x6BDA70
    // The map has _BeginFrameImpl(); this build passes BeginFrame's flags.
    virtual void _BeginFrameImpl(unsigned int flags);     // 0x6BDA80
    virtual void _SetRenderTargetsImpl(
        RndTargetMode mode,
        const RenderTargetParams& params) = 0;
    // The map has _SetBlendModeImpl(RndBlendMode); this build adds a blend
    // color.
    virtual void _SetBlendModeImpl(
        RndBlendMode mode,
        const Hmx::Color& blendColor) = 0;
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
    // Told the pipeline and compute slot that were active. The map has
    // _SetActivePipelineImpl(RndPipeline).
    virtual void _SetActivePipelineImpl(int previousPipeline, unsigned long previousSlot);  // 0x6BDA90
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
    // Selects the context's global constant buffers, falling back to the
    // device's defaults for the camera and the lights when none are set.
    void _ReselectGlobalCBuffers();  // 0x6BD930
    // Records on the graphics ring, or on the given compute context when
    // async compute is enabled. The map has SetActivePipeline(RndPipeline,
    // bool); this build passes the compute slot.
    void SetActivePipeline(RndPipeline pipeline, unsigned long computeSlot);  // 0x6BD8A0
    // Sets the camera of the main view and copies it to the second; the
    // stereo target modes then refresh its target info. Not reconstructed
    // yet.
    void SetCamera(const GameObject* camera);  // 0x6BD220
    // Draws with an identity view-projection, for screen-space geometry,
    // or with the camera's.
    void SetUsingIdentityViewProjection(bool identity);  // 0x6BD340
    // Wireframe shading also draws lines with depth bias.
    void SetShadingMode(RndShadingMode mode);  // 0x6BD5D0
    // Selects the camera constants for the current view-projection mode.
    // Not yet reconstructed.
    void _SyncCameraCBuffer();  // 0x6BCCD0
    // Resets the per-frame state, lets the platform start its frame, and
    // selects the default constant buffers. Name not in the reference map;
    // it may be the map's Reset().
    void BeginFrame(unsigned int flags);  // 0x6BC3B0
    // Writes the clip planes in the mask to the clip-plane buffer and
    // selects it, or the device's default when no plane is enabled.
    void _SyncClipPlanes(unsigned int mask);  // 0x6BC590

    // Statistic scopes opened on this context. Names not in the reference
    // map.
    RndGpuStatScope* LastGpuStatScope();
    void PushGpuStatScope(const RndGpuStatScope& scope);
    void PopGpuStatScope();

    // Field names are not in the reference map.
    bool mFrameActive;
    bool mDisableComputeQueues;
    int mMode;
    RndTargetMode mTargetMode;
    FixedVector<void*, 8> mUnknown24;
    // Set by the map's SetCameraCBufferOverrideContext.
    const RndCameraContext* mCameraCBufferOverride;
    unsigned char mUnknown120[8];
    // Size of the bound render targets, in pixels.
    float mRenderTargetWidth;
    float mRenderTargetHeight;
    unsigned char mUnknown136[4];
    float mUnknown140;
    RndCameraContext mCameras[2];
    bool mUsingIdentityViewProjection;
    unsigned long mUnknown18776;
    unsigned char mActiveShaderStages;  // A bit per stage with a program.
    RndBlendMode mBlendMode;
    unsigned long mInputSlotLimits[kNumShaderProgramTypes];
    unsigned long mOutputSlotLimits[kNumShaderProgramTypes];
    // Four user clip planes; an enabled one selects the context's clip-plane
    // constant buffer. Names not in the reference map.
    struct ClipPlane {
        bool mEnabled = false;
        float mPlane[4] = {0.0F, 0.0F, 1.0F, 0.0F};
    };
    ClipPlane mClipPlanes[4];
    RndShadingMode mShadingMode;
    int mUnknown18972;
    int mUnknown18976;
    // 0 records on the graphics context, 1 on a compute context.
    int mActivePipe;
    unsigned long mActiveComputeSlot;
    eastl::vector<RndContextRecord16> mUnknown18992;
    // Global constant buffers created by Init; the last three are the
    // per-draw buffers of 16, 32, and 64 elements.
    RndShaderCBuffer* mCBuffers[9];
    unsigned char mUnknown19096[3168];
    unsigned char mUnknown22264[8];
    eastl::vector<RndGpuStatScope> mGpuStatScopes;
    bool mUnknown22304;
};

static_assert(offsetof(RndContext, mTargetMode) == 0x10);
static_assert(offsetof(RndContext, mUnknown24) == 24);
static_assert(offsetof(RndContext, mCameras) == 144);
static_assert(offsetof(RndContext, mUsingIdentityViewProjection) == 18768);
static_assert(offsetof(RndContext, mRenderTargetWidth) == 128);
static_assert(offsetof(RndContext, mActiveShaderStages) == 0x4960);
static_assert(offsetof(RndContext, mBlendMode) == 0x4964);
static_assert(offsetof(RndContext, mInputSlotLimits) == 0x4968);
static_assert(offsetof(RndContext, mOutputSlotLimits) == 0x4998);
static_assert(sizeof(RndContext::ClipPlane) == 20);
static_assert(offsetof(RndContext, mClipPlanes) == 18888);
static_assert(offsetof(RndContext, mShadingMode) == 0x4A18);
static_assert(offsetof(RndContext, mActivePipe) == 0x4A24);
static_assert(offsetof(RndContext, mActiveComputeSlot) == 0x4A28);
static_assert(offsetof(RndContext, mUnknown18992) == 18992);
static_assert(offsetof(RndContext, mCBuffers) == 0x4A50);
static_assert(offsetof(RndContext, mUnknown19096) == 19096);
static_assert(offsetof(RndContext, mUnknown22264) == 0x56F8);
static_assert(offsetof(RndContext, mGpuStatScopes) == 0x5700);
static_assert(offsetof(RndContext, mUnknown22304) == 22304);
static_assert(sizeof(RndContext) == 0x5728);

static_assert(sizeof(RndContext::RenderTargetParams) == 288);
static_assert(offsetof(RndContext::RenderTargetParams, mTargets) == 48);
static_assert(offsetof(RndContext::RenderTargetParams, mDepthTexture) == 264);
