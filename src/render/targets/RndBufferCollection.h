#pragma once

#include <cstddef>

#include "math/vector/Vector2i.h"
#include "render/textures/RndPixelFormat.h"
#include "utl/containers/FixedVector.h"

class RndComputeBuffer;
class RndMesh;
class RndTexture3D;
class RndTextureBase;

// Buffers a collection creates. Names not in the reference map.
enum RndBufferCollectionFlags : unsigned int {
    kBufferDepthStencil = 0x00000002,
    kBufferLinearDepth = 0x00000004,
    kBufferLightAccum = 0x00000008,
    kBufferLightProbeAccum = 0x00000010,
    kBufferTiledLighting = 0x00000020,
    kBufferGBuffer = 0x00000040,
    kBufferAtmosphere = 0x00000080,
    kBufferDownsample = 0x00000100,
    kBufferSceneMask = 0x00000200,
    kBufferShadowBlur = 0x00000400,
    kBufferAO = 0x00000800,
    kBufferVolumetricScattering = 0x00001000,
    kBufferCMAA = 0x00002000,
    kBufferSceneMaskTiles = 0x00004000,
    kBufferForce64BitLightAccum = 0x20000000,
    kBufferPartialFramerate = 0x40000000,
    kBufferBackBufferNotOwned = 0x80000000,
};

// Render targets for one view: the back buffer and every intermediate buffer
// the passes draw into. The base vtable is at 0x1936DB8; the 2D and cube
// collections implement the allocation slots.
class RndBufferCollection {
public:
    // Per-frame state of a partial-framerate scene. Name and field names not
    // in the reference map.
    struct PartialFramerateData {
        int mUnknown0[5];
        unsigned int mUnknown20;
        int mUnknown24[4];
        unsigned int mUnknown40[4];
        unsigned int mUnknown56;
        bool mUnknown60;
        int mUnknown64[3];
        unsigned short mUnknown76;
        bool mUnknown78;
    };

    // Buffers drawn once per frame interval: the full-rate frame, then one
    // set per partial-framerate scene. Field names not in the reference map.
    struct FrameIntervalBuffers {
        PartialFramerateData* mPartialFramerateData;
        RndTextureBase* mPartialLightAccum;
        RndTextureBase* mDepthStencil;
        RndTextureBase* mUnknown24;
        RndTextureBase* mUnknown32;
        RndTextureBase* mGBufferColor;
        RndTextureBase* mGBufferPixelNormals;
        RndTextureBase* mGBufferVertexNormals;
        RndTextureBase* mLinearDepth;
        RndTextureBase* mTiledDepthRange;
        RndTextureBase* mAO;
        RndComputeBuffer* mTiledLightIds[2];
        RndComputeBuffer* mTiledLightIdRanges;
        RndTextureBase* mTiledLightInterp;
        RndComputeBuffer* mStereoTiledLightIds[2];
        RndComputeBuffer* mStereoTiledLightIdRanges;
        RndTexture3D* mVScatInscattering[3];
        RndTexture3D* mStereoVScatInscattering[3];
        RndTexture3D* mVScatAccumScattering[3];
    };

    // The map's constructor is RndBufferCollection(unsigned int).
    RndBufferCollection(unsigned int flags, int targetMode);  // 0x6AFEA0
    virtual ~RndBufferCollection();  // 0x6AFFC0, 0x6B0730

    virtual bool _ValidateBackBufferImpl(const RndTextureBase& backBuffer) = 0;  // slot 2
    // The map's signatures lack the data format, attachment index, and
    // reused texture that this build adds.
    virtual RndTextureBase* _AllocBufferImpl(
        const char* name,
        const RndPixelFormat& format,
        int dataFormat,
        const Vector2i& size,
        int attachment,
        unsigned int targetFlags,
        RndTextureBase* reuse) = 0;  // slot 3
    virtual RndTextureBase* _AllocBufferArrayImpl(
        const char* name,
        const RndPixelFormat& format,
        int dataFormat,
        const Vector2i& size,
        unsigned long count,
        int attachment,
        unsigned int targetFlags,
        RndTextureBase* reuse) = 0;  // slot 4

    void Destroy();  // 0x6AFFE0
    // Creates every buffer for the back buffer's size. The buffers of the
    // collection being replaced are offered to the allocator for reuse.
    void InstallBackBuffer(
        RndTextureBase* backBuffer,
        const RndBufferCollection* reuse);  // 0x6B0760
    // The map's parameter is RndTargetMode.
    void SetTargetMode(int mode);  // 0x6B28D0
    PartialFramerateData* ObtainPartialFramerateData(unsigned long scene);  // 0x6B2910
    void SelectPartialFramerateBuffers(long scene, long sceneContext);  // 0x6B2A20

    void _AllocLightAccumBuffers(const RndBufferCollection* reuse);   // 0x6B0B20
    void _AllocLightProbeAccumBuffer(const RndBufferCollection* reuse);
    void _AllocAtmosphereBuffers(const RndBufferCollection* reuse);   // 0x6B0E80
    void _AllocDownsampleBuffers(const RndBufferCollection* reuse);   // 0x6B12D0
    void _AllocSceneMaskBuffer(const RndBufferCollection* reuse);     // 0x6B1760
    void _AllocShadowBlurBuffers(const RndBufferCollection* reuse);   // 0x6B1970
    void _AllocCMAABuffers(const RndBufferCollection* reuse);         // 0x6B1E60
    // Name not in the reference map.
    void _AllocSceneMaskTileBuffers(const RndBufferCollection* reuse);  // 0x6B2140
    void _AllocFrameIntervalBuffers(
        FrameIntervalBuffers& buffers,
        bool partial,
        const FrameIntervalBuffers* reuse);  // 0x6B2660
    void _AllocDepthStencilBuffer(
        FrameIntervalBuffers& buffers,
        bool partial,
        const FrameIntervalBuffers* reuse);  // 0x6B2A80
    void _AllocLinearDepthBuffer(
        FrameIntervalBuffers& buffers,
        bool partial,
        const FrameIntervalBuffers* reuse);  // 0x6B2C90
    // The map's signature is _AllocOneLightAccumBuffer(char const*, long).
    RndTextureBase* _AllocOneLightAccumBuffer(
        const char* name,
        int halvings,
        long attachment,
        RndTextureBase* reuse,
        unsigned int targetFlags);  // 0x6B2E80
    void _AllocGBuffer(
        FrameIntervalBuffers& buffers,
        bool partial,
        const FrameIntervalBuffers* reuse);  // 0x6B3040
    void _AllocAOBuffers(
        FrameIntervalBuffers& buffers,
        bool partial,
        const FrameIntervalBuffers* reuse);  // 0x6B3270
    void _AllocTiledLightingBuffers(
        FrameIntervalBuffers& buffers,
        bool interp,
        bool stereo,
        RndTextureBase* reuseInterp);  // 0x6B3380
    void _AllocVolumetricScatteringBuffers(
        FrameIntervalBuffers& buffers,
        const FrameIntervalBuffers* reuse);  // 0x6B3730

    // Adds a buffer to the list that SetTargetMode updates. Name not in the
    // reference map.
    void _RegisterBuffer(RndTextureBase* buffer) {
        mBuffers.mData[mBuffers.mSize++] = buffer;
    }

    // Field names are not in the reference map.
    unsigned int mFlags;
    int mTargetMode;
    unsigned int mShadingMode;
    unsigned int mBufferInspectionMode;
    Vector2i mSize;
    FixedVector<RndTextureBase*, 38> mBuffers;
    unsigned long mNextAttachment;
    RndTextureBase* mBackBuffer;
    RndTextureBase* mLightAccum[2];
    RndTextureBase* mUnknown392;
    RndTextureBase* mBlurredLightAccum[3];
    RndTextureBase* mLightProbeAccum;
    RndTextureBase* mAtmosphere[4];
    RndTextureBase* mDownsample[3][2];
    RndTextureBase* mSceneMask;
    RndTextureBase* mSceneMaskScratch;
    RndTextureBase* mSceneMaskTile;
    unsigned long mCMAAState;
    RndTextureBase* mCMAAColor;
    RndTextureBase* mCMAAEdges[2];
    RndTextureBase* mCMAACompressedEdges;
    RndTextureBase* mShadowContribArray;
    RndTextureBase* mShadowContribStencil;
    RndTextureBase* mShadowContribScratch[2];
    RndTextureBase* mShadowSoftenTiles[2];
    RndTextureBase* mTiledSceneMask[2];
    RndMesh* mTiledSceneMaskMesh;
    FixedVector<FrameIntervalBuffers, 4> mFrameIntervals;
    unsigned long mActiveFrameInterval;
    long mActiveSceneContext;
};

static_assert(sizeof(RndBufferCollection::PartialFramerateData) == 80);
static_assert(sizeof(RndBufferCollection::FrameIntervalBuffers) == 216);
static_assert(
    offsetof(RndBufferCollection::FrameIntervalBuffers, mDepthStencil) == 0x10);
static_assert(
    offsetof(RndBufferCollection::FrameIntervalBuffers, mGBufferColor) == 0x28);
static_assert(offsetof(RndBufferCollection::FrameIntervalBuffers, mAO) == 0x50);
static_assert(
    offsetof(RndBufferCollection::FrameIntervalBuffers, mTiledLightIds) == 0x58);
static_assert(
    offsetof(RndBufferCollection::FrameIntervalBuffers, mVScatInscattering) ==
    0x90);
static_assert(
    offsetof(RndBufferCollection::FrameIntervalBuffers, mVScatAccumScattering) ==
    0xC0);
static_assert(offsetof(RndBufferCollection, mFlags) == 8);
static_assert(offsetof(RndBufferCollection, mShadingMode) == 16);
static_assert(offsetof(RndBufferCollection, mSize) == 0x18);
static_assert(offsetof(RndBufferCollection, mBuffers) == 0x20);
static_assert(offsetof(RndBufferCollection, mNextAttachment) == 0x168);
static_assert(offsetof(RndBufferCollection, mBackBuffer) == 0x170);
static_assert(offsetof(RndBufferCollection, mLightAccum) == 0x178);
static_assert(offsetof(RndBufferCollection, mAtmosphere) == 0x1B0);
static_assert(offsetof(RndBufferCollection, mDownsample) == 0x1D0);
static_assert(offsetof(RndBufferCollection, mSceneMask) == 0x200);
static_assert(offsetof(RndBufferCollection, mCMAAState) == 0x218);
static_assert(offsetof(RndBufferCollection, mCMAAColor) == 0x220);
static_assert(offsetof(RndBufferCollection, mShadowContribArray) == 0x240);
static_assert(offsetof(RndBufferCollection, mTiledSceneMask) == 0x270);
static_assert(offsetof(RndBufferCollection, mFrameIntervals) == 0x288);
static_assert(offsetof(RndBufferCollection, mActiveFrameInterval) == 0x600);
static_assert(offsetof(RndBufferCollection, mActiveSceneContext) == 0x608);
static_assert(sizeof(RndBufferCollection) == 0x610);

// Collection of 2D render targets. The vtable is at 0x1936DF0.
class RndBufferCollection2D : public RndBufferCollection {
public:
    // The map's constructor is RndBufferCollection2D(unsigned int).
    RndBufferCollection2D(unsigned int flags, int targetMode);  // 0x6B40A0
    ~RndBufferCollection2D() override;  // 0x6B40D0, 0x6B40E0

    bool _ValidateBackBufferImpl(const RndTextureBase& backBuffer) override;  // 0x6B4100
    RndTextureBase* _AllocBufferImpl(
        const char* name,
        const RndPixelFormat& format,
        int dataFormat,
        const Vector2i& size,
        int attachment,
        unsigned int targetFlags,
        RndTextureBase* reuse) override;  // 0x6B4120
    RndTextureBase* _AllocBufferArrayImpl(
        const char* name,
        const RndPixelFormat& format,
        int dataFormat,
        const Vector2i& size,
        unsigned long count,
        int attachment,
        unsigned int targetFlags,
        RndTextureBase* reuse) override;  // 0x6B41F0
};
