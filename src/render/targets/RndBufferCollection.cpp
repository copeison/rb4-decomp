#include "render/targets/RndBufferCollection.h"

#include <array>

#include "os/memory/MemMgr.h"
#include "render/buffers/RndComputeBuffer.h"
#include "render/system/RndCapabilities.h"
#include "render/system/RndConfig.h"
#include "render/textures/RndPixelFormat.h"
#include "render/meshes/RndMesh.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTexture3D.h"
#include "render/textures/RndTextureBase.h"

namespace {

constexpr unsigned long kBufferCapacity = 38;
constexpr unsigned long kFrameIntervalCapacity = 4;
constexpr std::size_t kCurrentPlatformConfig = 7;
constexpr int kNoAttachment = -1;
constexpr int kStereoTargetMode = 3;

const RndConfig& Settings() {
    return *TheRndDevice()->mSettings;
}

int ResolveFormat(const RndDataFormatInfo& descriptor) {
    return RndFindSupportedDataFormat(descriptor, kPlatformPS4);
}

// Format request shared by the render targets: wrap and filter modes over
// the common settings.
RndPixelFormat TargetFormat(
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags = 10) {
    RndPixelFormat format{};
    format.mSettings[5] = 1;
    format.mWrapMode = wrap;
    format.mFilterMode = filter;
    format.mFlags = flags;
    return format;
}

RndPixelFormat DefaultTargetFormat(unsigned int kind) {
    return TargetFormat(
        static_cast<unsigned int>(TextureDefaultWrapMode(kind)),
        static_cast<unsigned int>(TextureDefaultFilterMode(kind)));
}

int DivideRoundUp(int value, int divisor) {
    return value / divisor + (value % divisor != 0);
}

int Halve(int value, int halvings) {
    const auto scaled = value >> halvings;
    return scaled > 0 ? scaled : 1;
}

Vector2i HalveSize(const Vector2i& size, int halvings) {
    return {Halve(size.x, halvings), Halve(size.y, halvings)};
}

Vector2i TileCount(const Vector2i& size, int tileSize) {
    return {DivideRoundUp(size.x, tileSize), DivideRoundUp(size.y, tileSize)};
}

template <typename T>
void Release(T*& object) {
    if (object != nullptr) {
        delete object;
        object = nullptr;
    }
}

template <typename T>
T* Reused(const RndBufferCollection* reuse, T* RndBufferCollection::*field) {
    return reuse != nullptr ? reuse->*field : nullptr;
}

void ReleaseFrameInterval(RndBufferCollection::FrameIntervalBuffers& buffers) {
    if (buffers.mPartialFramerateData != nullptr) {
        operator delete(buffers.mPartialFramerateData);
        buffers.mPartialFramerateData = nullptr;
    }
    Release(buffers.mPartialLightAccum);
    Release(buffers.mDepthStencil);
    Release(buffers.mUnknown24);
    Release(buffers.mUnknown32);
    Release(buffers.mGBufferColor);
    Release(buffers.mGBufferPixelNormals);
    Release(buffers.mGBufferVertexNormals);
    Release(buffers.mLinearDepth);
    Release(buffers.mTiledDepthRange);
    Release(buffers.mAO);
    Release(buffers.mTiledLightIds[0]);
    Release(buffers.mStereoTiledLightIds[0]);
    Release(buffers.mTiledLightIds[1]);
    Release(buffers.mStereoTiledLightIds[1]);
    Release(buffers.mTiledLightIdRanges);
    Release(buffers.mTiledLightInterp);
    Release(buffers.mStereoTiledLightIdRanges);
    for (int i = 0; i < 3; ++i) {
        Release(buffers.mVScatInscattering[i]);
        Release(buffers.mStereoVScatInscattering[i]);
        Release(buffers.mVScatAccumScattering[i]);
    }
}

void ResizeFrameIntervals(
    FixedVector<RndBufferCollection::FrameIntervalBuffers, 4>& intervals,
    unsigned long count) {
    while (intervals.mSize < count) {
        intervals.mData[intervals.mSize++] = {};
    }
    intervals.mSize = count;
}

const RndBufferCollection::FrameIntervalBuffers* ReusedInterval(
    const RndBufferCollection* reuse,
    unsigned long index) {
    return reuse != nullptr ? &reuse->mFrameIntervals.mData[index] : nullptr;
}

RndBufferCollection::PartialFramerateData* NewPartialFramerateData() {
    auto* data = static_cast<RndBufferCollection::PartialFramerateData*>(
        operator new(sizeof(RndBufferCollection::PartialFramerateData)));
    *data = {};
    for (auto& value : data->mUnknown0) {
        value = -1;
    }
    for (auto& value : data->mUnknown24) {
        value = -1;
    }
    data->mUnknown40[2] = 33;
    data->mUnknown40[3] = 0xFFFFFFFFU;
    for (auto& value : data->mUnknown64) {
        value = -1;
    }
    return data;
}

RndComputeBuffer* NewTiledLightBuffer(
    unsigned long elementSize,
    unsigned long elementCount,
    const char* name) {
    RndComputeBuffer::Description desc{};
    desc.mElementSize = elementSize;
    desc.mNumElements = elementCount;
    desc.mFlags = 1;
    desc.mName = name;
    return RndComputeBuffer::New(desc);
}

// Each 32-bit light-id element holds two 16-bit ids; each range is 32 bytes.
void AllocTiledLightIds(
    RndComputeBuffer* (&ids)[2],
    RndComputeBuffer*& ranges,
    unsigned long tiles,
    unsigned long maxLightsPerTile,
    const char* idsName,
    const char* rangesName) {
    const auto idCount = tiles * maxLightsPerTile / 2;
    ids[0] = NewTiledLightBuffer(4, idCount, idsName);
    ids[1] = NewTiledLightBuffer(4, idCount, idsName);
    ranges = NewTiledLightBuffer(32, tiles, rangesName);
}

Vector2i TiledLightInterpSize(const Vector2i& size) {
    return {2 * DivideRoundUp(size.x, 2), DivideRoundUp(size.y, 2)};
}

// A full-rate tiled-light interpolation buffer reuses the previous
// collection's, or else the first shadow scratch buffer when it is large
// enough.
RndTextureBase* TiledLightInterpReuse(
    const RndBufferCollection& collection,
    const RndBufferCollection::FrameIntervalBuffers* reuse) {
    if (reuse != nullptr && reuse->mTiledLightInterp != nullptr) {
        return reuse->mTiledLightInterp;
    }
    auto* scratch = collection.mShadowContribScratch[0];
    if (scratch == nullptr) {
        return nullptr;
    }
    const auto size = TiledLightInterpSize(collection.mSize);
    return scratch->mBaseDesc.mWidth >= static_cast<unsigned int>(size.x) &&
            scratch->mBaseDesc.mHeight >= static_cast<unsigned int>(size.y)
        ? scratch
        : nullptr;
}

// Accumulated scattering starts as a checkerboard of two half-float voxels.
unsigned long AccumulatedScatteringVoxel(int x, int y, int z) {
    return ((x ^ y ^ z) & 1) == 0 ? 0x3C003C003C000000UL : 0x3C00000038003C00UL;
}

RndTexture3D* NewVScatTexture(
    const char* name,
    const Vector2i& size,
    int depth,
    RndTexture3D* reuse,
    bool checkerboard) {
    RndTexture3D::Description desc;
    desc.mType = RndTextureBase::kTexture3D;
    desc.mRequestedFormat.mWrapMode =
        static_cast<unsigned int>(TextureDefaultWrapMode(5));
    desc.mRequestedFormat.mFilterMode =
        static_cast<unsigned int>(TextureDefaultFilterMode(5));
    desc.mRequestedFormat.mFlags = 2;
    desc.mName = name;

    auto& pixels = desc.mPixels;
    pixels.mSize = {size.x, size.y, depth};
    pixels.mFormat = ResolveFormat({64, 4, 2, 1, -1});
    if (checkerboard) {
        // The binary allocates the voxels through RndPixelData::Create under
        // MemPushTemp; the description's destructor releases them.
        pixels.Create(pixels.mSize, pixels.mFormat, nullptr);
        auto* voxels = static_cast<unsigned long*>(pixels.mBuffer);
        for (int z = 0; z < depth; ++z) {
            for (int y = 0; y < size.y; ++y) {
                for (int x = 0; x < size.x; ++x) {
                    *voxels++ = AccumulatedScatteringVoxel(x, y, z);
                }
            }
        }
    }
    return RndTexture3D::New(desc, reuse);
}

}  // namespace

// Reconstructed from eboot.elf at 0x6AFEA0.
RndBufferCollection::RndBufferCollection(unsigned int flags, int targetMode)
    : mFlags(flags),
      mTargetMode(targetMode),
      mShadingMode(0),
      mBufferInspectionMode(0),
      mSize{0, 0},
      mNextAttachment(0),
      mBackBuffer(nullptr),
      mLightAccum{},
      mUnknown392(nullptr),
      mBlurredLightAccum{},
      mLightProbeAccum(nullptr),
      mAtmosphere{},
      mDownsample{},
      mSceneMask(nullptr),
      mSceneMaskScratch(nullptr),
      mSceneMaskTile(nullptr),
      mCMAAState(0),
      mCMAAColor(nullptr),
      mCMAAEdges{},
      mCMAACompressedEdges(nullptr),
      mShadowContribArray(nullptr),
      mShadowContribStencil(nullptr),
      mShadowContribScratch{},
      mShadowSoftenTiles{},
      mTiledSceneMask{},
      mTiledSceneMaskMesh(nullptr),
      mActiveFrameInterval(0),
      mActiveSceneContext(-1) {
    mBuffers.mData = mBuffers.mStorage;
    mBuffers.mSize = 0;
    mBuffers.mCapacity = kBufferCapacity;
    mFrameIntervals.mData = mFrameIntervals.mStorage;
    mFrameIntervals.mSize = 0;
    mFrameIntervals.mCapacity = kFrameIntervalCapacity;
}

// Reconstructed from eboot.elf at 0x6AFFC0.
RndBufferCollection::~RndBufferCollection() {
    Destroy();
}

// Reconstructed from eboot.elf at 0x6AFFE0.
void RndBufferCollection::Destroy() {
    if ((mFlags & kBufferBackBufferNotOwned) == 0 && mBackBuffer != nullptr) {
        delete mBackBuffer;
    }
    mBackBuffer = nullptr;

    for (auto*& buffer : mLightAccum) {
        Release(buffer);
    }
    for (auto*& buffer : mBlurredLightAccum) {
        Release(buffer);
    }
    Release(mUnknown392);
    Release(mLightProbeAccum);
    for (auto*& buffer : mAtmosphere) {
        Release(buffer);
    }
    for (int lane = 0; lane < 2; ++lane) {
        for (auto& level : mDownsample) {
            Release(level[lane]);
        }
    }
    Release(mSceneMask);
    Release(mSceneMaskScratch);
    Release(mSceneMaskTile);
    Release(mCMAAColor);
    Release(mCMAAEdges[0]);
    Release(mCMAAEdges[1]);
    Release(mCMAACompressedEdges);
    Release(mShadowContribArray);
    Release(mShadowContribStencil);
    Release(mShadowContribScratch[0]);
    Release(mShadowContribScratch[1]);
    Release(mShadowSoftenTiles[0]);
    Release(mShadowSoftenTiles[1]);
    Release(mTiledSceneMask[0]);
    Release(mTiledSceneMask[1]);
    Release(mTiledSceneMaskMesh);

    for (unsigned long i = 0; i < mFrameIntervals.mSize; ++i) {
        ReleaseFrameInterval(mFrameIntervals.mData[i]);
    }
    mFrameIntervals.mSize = 0;
    mSize = {0, 0};
    mNextAttachment = 0;
    mBuffers.mSize = 0;
}

// Reconstructed from eboot.elf at 0x6B0760.
void RndBufferCollection::InstallBackBuffer(
    RndTextureBase* backBuffer,
    const RndBufferCollection* reuse) {
    Destroy();
    mSize = {
        static_cast<int>(backBuffer->mBaseDesc.mWidth),
        static_cast<int>(backBuffer->mBaseDesc.mHeight),
    };
    static_cast<void>(_ValidateBackBufferImpl(*backBuffer));
    mBackBuffer = backBuffer;
    _RegisterBuffer(backBuffer);

    if ((mFlags & kBufferLightAccum) != 0) {
        _AllocLightAccumBuffers(reuse);
    }
    if ((mFlags & kBufferLightProbeAccum) != 0) {
        _AllocLightProbeAccumBuffer(reuse);
    }
    if ((mFlags & kBufferAtmosphere) != 0) {
        _AllocAtmosphereBuffers(reuse);
    }
    if ((mFlags & kBufferDownsample) != 0) {
        _AllocDownsampleBuffers(reuse);
    }
    if ((mFlags & kBufferSceneMask) != 0) {
        _AllocSceneMaskBuffer(reuse);
    }
    if ((mFlags & kBufferShadowBlur) != 0) {
        _AllocShadowBlurBuffers(reuse);
    }
    if ((mFlags & kBufferCMAA) != 0) {
        _AllocCMAABuffers(reuse);
    }
    if ((mFlags & kBufferSceneMaskTiles) != 0) {
        _AllocSceneMaskTileBuffers(reuse);
    }

    ResizeFrameIntervals(mFrameIntervals, 1);
    _AllocFrameIntervalBuffers(
        mFrameIntervals.mData[0], false, ReusedInterval(reuse, 0));

    if ((mFlags & kBufferPartialFramerate) != 0) {
        const auto scenes =
            static_cast<unsigned long>(Settings().mMaxPartialFramerateScenes);
        ResizeFrameIntervals(mFrameIntervals, scenes + 1);
        for (unsigned long i = 1; i <= scenes; ++i) {
            _AllocFrameIntervalBuffers(
                mFrameIntervals.mData[i], true, ReusedInterval(reuse, i));
        }
    }

    for (unsigned long i = 0; i < mBuffers.mSize; ++i) {
        mBuffers.mData[i]->mResourceIndex = mTargetMode;
    }
}

// Reconstructed from eboot.elf at 0x6B0B20.
void RndBufferCollection::_AllocLightAccumBuffers(
    const RndBufferCollection* reuse) {
    constexpr const char* kName = "Light Accum Buffer";
    constexpr const char* kBlurredName = "Blurred Light Accum Buffer";
    mLightAccum[0] = _AllocOneLightAccumBuffer(
        kName, 0, 0, reuse != nullptr ? reuse->mLightAccum[0] : nullptr, 0);
    _RegisterBuffer(mLightAccum[0]);
    mLightAccum[1] = _AllocOneLightAccumBuffer(
        kName, 0, 1, reuse != nullptr ? reuse->mLightAccum[1] : nullptr, 0);
    _RegisterBuffer(mLightAccum[1]);
    for (int i = 0; i < 3; ++i) {
        mBlurredLightAccum[i] = _AllocOneLightAccumBuffer(
            kBlurredName,
            i + 1,
            -1,
            reuse != nullptr ? reuse->mBlurredLightAccum[i] : nullptr,
            0);
        _RegisterBuffer(mBlurredLightAccum[i]);
    }
}

// Reconstructed from the light-probe branch of eboot.elf at 0x6B0760.
void RndBufferCollection::_AllocLightProbeAccumBuffer(
    const RndBufferCollection* reuse) {
    if (Settings().mUseTiledLighting) {
        return;
    }
    mLightProbeAccum = _AllocBufferImpl(
        "Light Probe Accum Buffer",
        TargetFormat(1, 1),
        ResolveFormat({64, 4, 2, 1, -1}),
        mSize,
        kNoAttachment,
        0,
        Reused(reuse, &RndBufferCollection::mLightProbeAccum));
    _RegisterBuffer(mLightProbeAccum);
}

// Reconstructed from eboot.elf at 0x6B0E80. Without a previous collection,
// the reduced levels reuse the new full-size buffer.
void RndBufferCollection::_AllocAtmosphereBuffers(
    const RndBufferCollection* reuse) {
    auto format = TargetFormat(1, 1);
    const auto dataFormat = ResolveFormat({32, 4, 1, 1, -1});
    for (int level = 0; level < 4; ++level) {
        auto* reused = reuse != nullptr
            ? reuse->mAtmosphere[level]
            : (level == 0 ? nullptr : mAtmosphere[0]);
        format.mFilterMode = level == 0 ? 1 : 2;
        mAtmosphere[level] = _AllocBufferImpl(
            "Sky Buffer",
            format,
            dataFormat,
            HalveSize(mSize, level),
            kNoAttachment,
            0,
            reused);
        _RegisterBuffer(mAtmosphere[level]);
    }
}

// Reconstructed from eboot.elf at 0x6B12D0.
void RndBufferCollection::_AllocDownsampleBuffers(
    const RndBufferCollection* reuse) {
    static const char* const kNames[3] = {
        "Half-Size Buffer",
        "Quarter-Size Buffer",
        "Eighth-Size Buffer",
    };
    const bool wide = Settings().mUse64BitLightAccum;
    const auto dataFormat =
        ResolveFormat({wide ? 64U : 32U, wide ? 4U : 2U, 2, 1, -1});
    const auto format = TargetFormat(1, 2);
    for (int level = 0; level < 3; ++level) {
        const auto size = HalveSize(mSize, level + 1);
        for (int lane = 0; lane < 2; ++lane) {
            auto*& buffer = mDownsample[level][lane];
            buffer = _AllocBufferImpl(
                kNames[level],
                format,
                dataFormat,
                size,
                kNoAttachment,
                0,
                reuse != nullptr ? reuse->mDownsample[level][lane] : nullptr);
            _RegisterBuffer(buffer);
        }
    }
}

// Reconstructed from eboot.elf at 0x6B1760.
void RndBufferCollection::_AllocSceneMaskBuffer(
    const RndBufferCollection* reuse) {
    auto format = DefaultTargetFormat(32);
    const auto dataFormat = ResolveFormat({8, 10, 0, 1, -1});
    mSceneMask = _AllocBufferImpl(
        "Mask Buffer",
        format,
        dataFormat,
        mSize,
        kNoAttachment,
        0,
        Reused(reuse, &RndBufferCollection::mSceneMask));
    _RegisterBuffer(mSceneMask);
    mSceneMaskScratch = _AllocBufferImpl(
        "Mask Scratch Buffer",
        format,
        dataFormat,
        mSize,
        kNoAttachment,
        0,
        Reused(reuse, &RndBufferCollection::mSceneMaskScratch));
    _RegisterBuffer(mSceneMaskScratch);

    format.mFilterMode = 1;
    mSceneMaskTile = _AllocBufferImpl(
        "Mask Tile Buffer",
        format,
        dataFormat,
        TileCount(mSize, static_cast<int>(Settings().mMaskTileSize)),
        kNoAttachment,
        0,
        Reused(reuse, &RndBufferCollection::mSceneMaskTile));
    _RegisterBuffer(mSceneMaskTile);
}

// Reconstructed from eboot.elf at 0x6B1970. Above 1080p the contribution
// buffers are half size and gain a stencil and a second scratch buffer.
void RndBufferCollection::_AllocShadowBlurBuffers(
    const RndBufferCollection* reuse) {
    const auto& settings = Settings();
    if (settings.mMaxShadowContribBuffers == 0) {
        return;
    }

    const bool reduced = mSize.x > 1920 || mSize.y > 1080;
    const Vector2i size = reduced ? Vector2i{mSize.x / 2, mSize.y / 2} : mSize;

    auto format = DefaultTargetFormat(27);
    mShadowContribArray = _AllocBufferArrayImpl(
        "Shadow Contrib TexArray",
        format,
        ResolveFormat({8, 10, 0, 1, -1}),
        size,
        static_cast<unsigned long>(settings.mMaxShadowContribBuffers),
        kNoAttachment,
        0,
        Reused(reuse, &RndBufferCollection::mShadowContribArray));
    _RegisterBuffer(mShadowContribArray);

    format.mFilterMode = 1;
    if (reduced) {
        auto stencilFormat = format;
        stencilFormat.mUsage = kTextureUsageDepth;
        mShadowContribStencil = _AllocBufferImpl(
            "Shadow Contrib Stencil",
            stencilFormat,
            ResolveFormat({24, 11, 0, 1, -1}),
            size,
            kNoAttachment,
            16,
            Reused(reuse, &RndBufferCollection::mShadowContribStencil));
        _RegisterBuffer(mShadowContribStencil);
    }

    const auto scratchFormat = ResolveFormat({64, 4, 2, 1, -1});
    const int scratchCount = reduced ? 2 : 1;
    for (int i = 0; i < scratchCount; ++i) {
        mShadowContribScratch[i] = _AllocBufferImpl(
            "Shadow Contrib Scratch",
            format,
            scratchFormat,
            size,
            kNoAttachment,
            0,
            reuse != nullptr ? reuse->mShadowContribScratch[i] : nullptr);
        _RegisterBuffer(mShadowContribScratch[i]);
    }

    const auto tiles =
        TileCount(size, static_cast<int>(settings.mShadowSoftenTileSize));
    const auto tileFormat = ResolveFormat({8, 10, 0, 1, -1});
    for (int i = 0; i < 2; ++i) {
        mShadowSoftenTiles[i] = _AllocBufferImpl(
            "Shadow Soften Tiles",
            format,
            tileFormat,
            tiles,
            kNoAttachment,
            0,
            reuse != nullptr ? reuse->mShadowSoftenTiles[i] : nullptr);
        _RegisterBuffer(mShadowSoftenTiles[i]);
    }
}

// Reconstructed from eboot.elf at 0x6B1E60. The color buffer is created only
// when the previous collection had one.
void RndBufferCollection::_AllocCMAABuffers(const RndBufferCollection* reuse) {
    if ((TheRndDevice()->mCapabilities[kCurrentPlatformConfig].mFeatureFlags &
         0x10U) == 0) {
        return;
    }

    auto format = TargetFormat(1, 2);
    const bool wide = Settings().mUse64BitLightAccum;
    if (reuse != nullptr && reuse->mCMAAColor != nullptr) {
        mCMAAColor = _AllocBufferImpl(
            "CMAA Color Buffer",
            format,
            ResolveFormat({wide ? 64U : 32U, wide ? 4U : 2U, 2, 1, -1}),
            mSize,
            kNoAttachment,
            0,
            reuse->mCMAAColor);
        if (mCMAAColor != nullptr) {
            _RegisterBuffer(mCMAAColor);
        }
    } else {
        mCMAAColor = nullptr;
    }

    format.mFilterMode = 1;
    const auto edgeFormat = ResolveFormat({8, 10, 0, 1, -1});
    for (int i = 0; i < 2; ++i) {
        mCMAAEdges[i] = _AllocBufferImpl(
            "CMAA Edge Buffer",
            format,
            edgeFormat,
            mSize,
            kNoAttachment,
            0,
            reuse != nullptr ? reuse->mCMAAEdges[i] : nullptr);
        _RegisterBuffer(mCMAAEdges[i]);
    }

    mCMAACompressedEdges = _AllocBufferImpl(
        "CMAA Compressed Edge Buffer",
        format,
        ResolveFormat({32, 4, 3, 1, -1}),
        HalveSize(mSize, 1),
        kNoAttachment,
        0,
        Reused(reuse, &RndBufferCollection::mCMAACompressedEdges));
    _RegisterBuffer(mCMAACompressedEdges);
    mCMAAState = 0;
}

// Reconstructed from eboot.elf at 0x6B2140. One quad per light tile, in clip
// space.
void RndBufferCollection::_AllocSceneMaskTileBuffers(
    const RndBufferCollection* reuse) {
    const auto tileSize = static_cast<int>(Settings().mLightTileSize);
    const auto tiles = TileCount(mSize, tileSize);
    const auto format = TargetFormat(1, 1);
    const auto dataFormat = ResolveFormat({8, 10, 0, 1, -1});
    for (int i = 0; i < 2; ++i) {
        mTiledSceneMask[i] = _AllocBufferImpl(
            "Scene Mask",
            format,
            dataFormat,
            tiles,
            kNoAttachment,
            i == 0 ? 4 : 0,
            reuse != nullptr ? reuse->mTiledSceneMask[i] : nullptr);
        _RegisterBuffer(mTiledSceneMask[i]);
    }

    auto* mesh = RndMesh::New(kVertexPosOnly, "Scene Mask Mesh");
    const auto tileCount = static_cast<unsigned long>(tiles.x) * tiles.y;
    mesh->_SetNumVerticesImpl(tileCount * 4);
    mesh->mFaces.resize(tileCount * 2);

    unsigned long tile = 0;
    for (int row = 0; row < tiles.y; ++row) {
        const auto top = row * tileSize;
        const auto bottom =
            (row + 1) * tileSize < mSize.y ? (row + 1) * tileSize : mSize.y;
        const auto y0 = 1.0F - 2.0F * static_cast<float>(top) / mSize.y;
        const auto y1 = 1.0F - 2.0F * static_cast<float>(bottom) / mSize.y;
        for (int column = 0; column < tiles.x; ++column, ++tile) {
            const auto left = column * tileSize;
            const auto right = (column + 1) * tileSize < mSize.x
                ? (column + 1) * tileSize
                : mSize.x;
            const auto x0 = 2.0F * static_cast<float>(left) / mSize.x - 1.0F;
            const auto x1 = 2.0F * static_cast<float>(right) / mSize.x - 1.0F;

            const auto vertex = static_cast<unsigned int>(tile * 4);
            auto position = [mesh](unsigned long index) -> RndVertexPosOnly& {
                return *static_cast<RndVertexPosOnly*>(
                    mesh->_GetVertexVoidImpl(index));
            };
            position(vertex + 0) = {{x0, y0, 0.0F}};
            position(vertex + 1) = {{x1, y0, 0.0F}};
            position(vertex + 2) = {{x0, y1, 0.0F}};
            position(vertex + 3) = {{x1, y1, 0.0F}};
            mesh->mFaces[tile * 2 + 0] = {{vertex + 0, vertex + 1, vertex + 2}};
            mesh->mFaces[tile * 2 + 1] = {{vertex + 1, vertex + 3, vertex + 2}};
        }
    }
    mesh->SyncStatic();
    mTiledSceneMaskMesh = mesh;
}

// Reconstructed from eboot.elf at 0x6B2660.
void RndBufferCollection::_AllocFrameIntervalBuffers(
    FrameIntervalBuffers& buffers,
    bool partial,
    const FrameIntervalBuffers* reuse) {
    if (partial) {
        buffers.mPartialFramerateData = NewPartialFramerateData();
        if ((mFlags & kBufferLightAccum) != 0) {
            buffers.mPartialLightAccum = _AllocOneLightAccumBuffer(
                "Partial Light Accum Buffer",
                0,
                -1,
                reuse != nullptr ? reuse->mPartialLightAccum : nullptr,
                0);
        }
    }

    if ((mFlags & kBufferDepthStencil) != 0) {
        _AllocDepthStencilBuffer(buffers, partial, reuse);
    }
    if ((mFlags & kBufferGBuffer) != 0) {
        _AllocGBuffer(buffers, partial, reuse);
    }
    if ((mFlags & kBufferLinearDepth) != 0) {
        _AllocLinearDepthBuffer(buffers, partial, reuse);
    }
    if ((mFlags & kBufferAO) != 0) {
        _AllocAOBuffers(buffers, partial, reuse);
    }
    if ((mFlags & kBufferTiledLighting) != 0) {
        _AllocTiledLightingBuffers(
            buffers,
            !partial,
            mTargetMode == kStereoTargetMode,
            partial ? nullptr : TiledLightInterpReuse(*this, reuse));
    }
    if ((mFlags & kBufferVolumetricScattering) != 0) {
        _AllocVolumetricScatteringBuffers(buffers, reuse);
    }
}

// Reconstructed from eboot.elf at 0x6B28D0.
void RndBufferCollection::SetTargetMode(int mode) {
    if (mTargetMode != mode) {
        mTargetMode = mode;
        for (unsigned long i = 0; i < mBuffers.mSize; ++i) {
            mBuffers.mData[i]->mResourceIndex = mTargetMode;
        }
    }
}

// Reconstructed from eboot.elf at 0x6B2910. Missing partial-framerate scenes
// are created on demand.
RndBufferCollection::PartialFramerateData*
RndBufferCollection::ObtainPartialFramerateData(unsigned long scene) {
    const auto index = scene + 1;
    const auto count = mFrameIntervals.mSize;
    if (index >= count) {
        ResizeFrameIntervals(mFrameIntervals, index + 1);
        for (auto i = count; i <= index; ++i) {
            _AllocFrameIntervalBuffers(mFrameIntervals.mData[i], true, nullptr);
        }
    }
    return mFrameIntervals.mData[index].mPartialFramerateData;
}

// Reconstructed from eboot.elf at 0x6B2A20.
void RndBufferCollection::SelectPartialFramerateBuffers(
    long scene,
    long sceneContext) {
    mActiveFrameInterval = static_cast<unsigned long>(scene + 1);
    mActiveSceneContext = sceneContext;
}

// Reconstructed from eboot.elf at 0x6B2A80. A reused buffer keeps no
// attachment; a reused buffer with one is replaced.
void RndBufferCollection::_AllocDepthStencilBuffer(
    FrameIntervalBuffers& buffers,
    bool partial,
    const FrameIntervalBuffers* reuse) {
    auto* reused = reuse != nullptr ? reuse->mDepthStencil : nullptr;
    int attachment =
        partial ? kNoAttachment : static_cast<int>(mNextAttachment);
    if (!partial && reused != nullptr) {
        if (reused->mBaseDesc.mAttachmentIndex == kNoAttachment) {
            attachment = kNoAttachment;
        } else {
            reused = nullptr;
        }
    }

    const bool wide = Settings().mUse40BitDepthStencil;
    auto format = TargetFormat(1, 1, 2);
    format.mUsage = kTextureUsageDepth;
    auto* buffer = _AllocBufferImpl(
        "Depth/Stencil Buffer",
        format,
        ResolveFormat({wide ? 40U : 32U, 11, wide ? 2U : 0U, 1, -1}),
        mSize,
        attachment,
        16,
        reused);
    buffers.mDepthStencil = buffer;
    if (buffer->mBaseDesc.mAttachmentIndex != kNoAttachment) {
        mNextAttachment = static_cast<unsigned long>(
            buffer->mBaseDesc.mAttachmentIndex +
            buffer->mBaseDesc.mAttachmentCount);
    }
    if (!partial) {
        _RegisterBuffer(buffer);
    }
}

// Reconstructed from eboot.elf at 0x6B2C90.
void RndBufferCollection::_AllocLinearDepthBuffer(
    FrameIntervalBuffers& buffers,
    bool partial,
    const FrameIntervalBuffers* reuse) {
    const auto format = DefaultTargetFormat(7);
    buffers.mLinearDepth = _AllocBufferImpl(
        "Linear Depth Buffer",
        format,
        ResolveFormat({16, 10, 0, 1, -1}),
        mSize,
        kNoAttachment,
        0,
        reuse != nullptr ? reuse->mLinearDepth : nullptr);
    if (!partial) {
        _RegisterBuffer(buffers.mLinearDepth);
    }

    const auto& settings = Settings();
    if (!settings.mUseTiledLighting) {
        return;
    }
    buffers.mTiledDepthRange = _AllocBufferImpl(
        "Tiled Depth Range",
        format,
        ResolveFormat({32, 0, 0, 1, -1}),
        TileCount(mSize, static_cast<int>(settings.mLightTileSize)),
        kNoAttachment,
        0,
        reuse != nullptr ? reuse->mTiledDepthRange : nullptr);
    if (!partial) {
        _RegisterBuffer(buffers.mTiledDepthRange);
    }
}

// Reconstructed from eboot.elf at 0x6B2E80. A non-negative attachment
// selector takes the next attachment slot.
RndTextureBase* RndBufferCollection::_AllocOneLightAccumBuffer(
    const char* name,
    int halvings,
    long attachment,
    RndTextureBase* reuse,
    unsigned int targetFlags) {
    const bool wide = (mFlags & kBufferForce64BitLightAccum) != 0 ||
        Settings().mUse64BitLightAccum;
    auto size = mSize;
    for (; halvings > 0; --halvings) {
        size.x = size.x / 2 > 0 ? size.x / 2 : 1;
        size.y = size.y / 2 > 0 ? size.y / 2 : 1;
    }

    auto* buffer = _AllocBufferImpl(
        name,
        DefaultTargetFormat(18),
        ResolveFormat({wide ? 64U : 32U, wide ? 4U : 2U, 2, 1, -1}),
        size,
        attachment < 0 ? kNoAttachment : static_cast<int>(mNextAttachment),
        targetFlags,
        reuse);
    if (buffer->mBaseDesc.mAttachmentIndex != kNoAttachment) {
        mNextAttachment = static_cast<unsigned long>(
            buffer->mBaseDesc.mAttachmentIndex +
            buffer->mBaseDesc.mAttachmentCount);
    }
    return buffer;
}

// Reconstructed from eboot.elf at 0x6B3040.
void RndBufferCollection::_AllocGBuffer(
    FrameIntervalBuffers& buffers,
    bool partial,
    const FrameIntervalBuffers* reuse) {
    const auto format = DefaultTargetFormat(31);
    buffers.mGBufferColor = _AllocBufferImpl(
        "GBuffer Color",
        format,
        ResolveFormat({32, 4, 0, 2, -1}),
        mSize,
        kNoAttachment,
        0,
        reuse != nullptr ? reuse->mGBufferColor : nullptr);
    if (!partial) {
        _RegisterBuffer(buffers.mGBufferColor);
    }

    const auto normalFormat = ResolveFormat({32, 4, 1, 1, -1});
    buffers.mGBufferPixelNormals = _AllocBufferImpl(
        "GBuffer Pixel Normals",
        format,
        normalFormat,
        mSize,
        kNoAttachment,
        0,
        reuse != nullptr ? reuse->mGBufferPixelNormals : nullptr);
    if (!partial) {
        _RegisterBuffer(buffers.mGBufferPixelNormals);
    }

    if (!Settings().mUseGBufferVertexNormals) {
        buffers.mGBufferVertexNormals = nullptr;
        return;
    }
    buffers.mGBufferVertexNormals = _AllocBufferImpl(
        "GBuffer Vertex Normals",
        format,
        normalFormat,
        mSize,
        kNoAttachment,
        0,
        reuse != nullptr ? reuse->mGBufferVertexNormals : nullptr);
    if (!partial) {
        _RegisterBuffer(buffers.mGBufferVertexNormals);
    }
}

// Reconstructed from eboot.elf at 0x6B3270, which is also inlined into
// _AllocFrameIntervalBuffers.
void RndBufferCollection::_AllocAOBuffers(
    FrameIntervalBuffers& buffers,
    bool partial,
    const FrameIntervalBuffers* reuse) {
    buffers.mAO = _AllocBufferImpl(
        "AO Buffer",
        DefaultTargetFormat(28),
        ResolveFormat({32, 10, 2, 1, -1}),
        mSize,
        kNoAttachment,
        0,
        reuse != nullptr ? reuse->mAO : nullptr);
    if (!partial) {
        _RegisterBuffer(buffers.mAO);
    }
}

// Reconstructed from eboot.elf at 0x6B3380.
void RndBufferCollection::_AllocTiledLightingBuffers(
    FrameIntervalBuffers& buffers,
    bool interp,
    bool stereo,
    RndTextureBase* reuseInterp) {
    const auto& settings = Settings();
    if (!settings.mUseTiledLighting) {
        return;
    }

    const auto tileSize = static_cast<int>(settings.mLightTileSize);
    const auto tiles = static_cast<unsigned long>(DivideRoundUp(mSize.x, tileSize)) *
        static_cast<unsigned long>(DivideRoundUp(mSize.y, tileSize)) *
        static_cast<unsigned long>(settings.mLightTileDepthSlices);
    const auto maxLights = static_cast<unsigned long>(settings.mMaxLightsPerTile);

    AllocTiledLightIds(
        buffers.mTiledLightIds,
        buffers.mTiledLightIdRanges,
        tiles,
        maxLights,
        "Tiled Light Ids",
        "Tiled Light Id Ranges");

    if (interp) {
        buffers.mTiledLightInterp = _AllocBufferImpl(
            "Tiled Light Interp",
            TargetFormat(1, 2),
            ResolveFormat({64, 4, 2, 1, -1}),
            TiledLightInterpSize(mSize),
            kNoAttachment,
            0,
            reuseInterp);
    }

    if (stereo) {
        AllocTiledLightIds(
            buffers.mStereoTiledLightIds,
            buffers.mStereoTiledLightIdRanges,
            tiles,
            maxLights,
            "Tiled Light Ids (Both Eyes)",
            "Tiled Light Id Ranges (Both Eyes)");
    }
}

// Reconstructed from eboot.elf at 0x6B3730. Each kind has 512-, 256-, and
// 128-deep volumes; without a previous collection the shallower volumes
// reuse the new 512-deep one.
void RndBufferCollection::_AllocVolumetricScatteringBuffers(
    FrameIntervalBuffers& buffers,
    const FrameIntervalBuffers* reuse) {
    const auto& settings = Settings();
    if (!settings.mVolumetricScatteringEnabled) {
        return;
    }

    const auto tileSize =
        static_cast<int>(settings.mVolumetricScatteringTileSize);
    const Vector2i size{
        DivideRoundUp(DivideRoundUp(mSize.x, tileSize), 8) * 8,
        DivideRoundUp(DivideRoundUp(mSize.y, tileSize), 8) * 8,
    };

    // Indices 0, 1, 2 hold the 128-, 256-, and 512-deep volumes.
    for (int index = 2; index >= 0; --index) {
        const int depth = 128 << index;
        buffers.mVScatInscattering[index] = NewVScatTexture(
            "VScat Inscattering",
            size,
            depth,
            reuse != nullptr ? reuse->mVScatInscattering[index]
                : (index == 2 ? nullptr : buffers.mVScatInscattering[2]),
            false);
        buffers.mVScatAccumScattering[index] = NewVScatTexture(
            "VScat Accum Scattering",
            size,
            depth,
            reuse != nullptr ? reuse->mVScatAccumScattering[index]
                : (index == 2 ? nullptr : buffers.mVScatAccumScattering[2]),
            true);
    }

    if (mTargetMode == kStereoTargetMode) {
        for (int index = 0; index < 3; ++index) {
            buffers.mStereoVScatInscattering[index] = NewVScatTexture(
                "VScat Inscattering (Stereo)",
                size,
                128 << index,
                reuse != nullptr ? reuse->mStereoVScatInscattering[index]
                                 : nullptr,
                false);
        }
    }
}
