// render/RndLightMgrCom.o (0x480280 to 0x48ED8F). Slot 30 (0x480D80), the
// probe capture (0x48BB20-0x48C4F5) and the registry (0x486830) are not
// reconstructed.
#include "render/lighting/RndLightMgrCom.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <limits>
#include <new>

#include "entity/core/Entity.h"
#include "math/vector/Vector3i.h"
#include "os/memory/MemMgr.h"
#include "math/geometry/Frustum.h"
#include "audio/core/modulation/ModulatorTarget.h"
#include "entity/progress/LoadProgress.h"
#include "render/buffers/RndCShaderClearBuffer.h"
#include "render/buffers/RndComputeBuffer.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndDrawUtl.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/drawing/RndSceneDrawer.h"
#include "render/lighting/RndLightGlobals.h"
#include "render/lighting/RndLightUtl.h"
#include "render/lighting/ambient_occlusion/RndSSAOCom.h"
#include "render/lighting/deferred/RndLightDirectionalDeferredShader.h"
#include "render/lighting/deferred/RndLightProbeDeferredAccumShader.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/lighting/lights/RndLightEnvironCom.h"
#include "render/lighting/lights/RndLightProbeCom.h"
#include "render/lighting/tiled/RndCShaderTiledLightsApplication.h"
#include "render/lighting/tiled/RndCShaderTiledLightsCull.h"
#include "render/lighting/tiled/RndCShaderTiledLightsInterpolation.h"
#include "render/lighting/tiled/RndCShaderTiledLightsStereoToMono.h"
#include "render/postprocessing/tonemap/RndTonemapShader.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/system/RndFence.h"
#include "render/targets/RndBufferCollection.h"
#include "render/targets/RndScenePartialFramerateData.h"
#include "render/textures/RndPixelData.h"
#include "render/textures/RndPixelDataCube.h"
#include "render/textures/RndPixelFormat.h"
#include "render/textures/RndTextureArray2D.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTextureCube.h"
#include "render/textures/RndTextureArrayCube.h"
#include "render/textures/RndTextureBase.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x1A882F8, which the static initializer
// (0x48ECC0) sets with the shared header's thread-group widths. Names not
// in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};
[[maybe_unused]] int gLightMgrGroupSize2D = 8;  // 0x1A882FC
[[maybe_unused]] int gLightMgrGroupSize3D = 4;  // 0x1A88300

constexpr std::uint32_t kTiledLightBufferFlags = 0x12;
// RndCapabilities::mFeatureFlags: the platform runs compute asynchronously.
constexpr std::uint32_t kAsyncComputeFeature = 0x10;
// The stencil modes and read mask of the lighting quads: the tonemap and
// post-processing quads test the scene's stencil, the probes add where
// lights were drawn.
constexpr unsigned int kSceneStencilMode = 2;
constexpr unsigned int kSceneStencilReadMask = 6;
constexpr unsigned int kProbeStencilMode = 5;
// More environments than this are not indexed.
constexpr unsigned long kMaxIndexedEnvirons = 6;

constexpr std::array<std::uint32_t, 7> kSpotShadowResolutions{
    256,
    512,
    1024,
    1600,
    2048,
    3200,
    4096,
};

RndComputeBuffer* NewTiledLightBuffer(
    unsigned long elementSize,
    unsigned long numElements,
    const char* name) {
    RndComputeBuffer::Description desc{};
    desc.mElementSize = elementSize;
    desc.mNumElements = numElements;
    desc.mFlags = kTiledLightBufferFlags;
    desc.mName = name;
    return RndComputeBuffer::New(desc);
}

template <class T>
void SafeDelete(T*& object) {
    delete object;
    object = nullptr;
}

// Removes the element at the index, as the binary inlines it.
void RemoveAt(PropArrayBase& array, unsigned long index) {
    array.Destruct(1, array.ElementAt(index));
    if (index < array.mSize - 1) {
        array.Move(array.mSize - 1 - index, array.ElementAt(index), array.ElementAt(index + 1));
    }
    --array.mSize;
}

// The index of the id in the array, or -1.
unsigned long Find(const PropArray<GameObjectId>& array, GameObjectId id) {
    for (unsigned long i = 0; i < array.size(); ++i) {
        if (array[i].mId == id.mId) {
            return i;
        }
    }
    return static_cast<unsigned long>(-1);
}

unsigned long Find(const PropArray<Symbol>& array, Symbol name) {
    for (unsigned long i = 0; i < array.size(); ++i) {
        if (array[i] == name) {
            return i;
        }
    }
    return static_cast<unsigned long>(-1);
}

template <class T>
void EraseElement(eastl::vector<T*>& list, T* element) {
    T** position = list.begin();
    while (position != list.end() && *position != element) {
        ++position;
    }
    list.erase(position);
}

// Whether a shading mode draws lights, and whether it draws probes: the
// unlit and mask modes, among others, do not. Names not in the reference
// map.
bool ShadingModeKeepsLights(unsigned int mode) {
    if (mode - 18 < 8) {
        return false;
    }
    if (mode - 1 <= 30 && ((0x7E00001Fu >> (mode - 1)) & 1) != 0) {
        return false;
    }
    return !(mode - 12 <= 5 && ((0x27u >> (mode - 12)) & 1) != 0);
}

bool ShadingModeKeepsProbes(unsigned int mode) {
    if (mode - 18 < 8) {
        return false;
    }
    if (mode - 1 <= 30 && ((0x7E00001Fu >> (mode - 1)) & 1) != 0) {
        return false;
    }
    return !(mode <= 16 && ((0x10E00u >> mode) & 1) != 0);
}

void Clear(RndLightMgrCom::CullResults& results) {
    for (auto& lights : results.mLights) {
        lights[0].clear();
        lights[1].clear();
    }
    results.mProbes.clear();
    results.mShadowLights.clear();
}

// The corner part of the way from the near corner to the far one.
Vector3 LerpCorner(const Vector3& near, const Vector3& far, float t) {
    if (t == 1.0F) {
        return far;
    }
    return Vector3{
        (far.x - near.x) * t + near.x,
        (far.y - near.y) * t + near.y,
        (far.z - near.z) * t + near.z,
    };
}

// Resource states the passes move their buffers between: read by any
// shader stage, read by the non-pixel stages, and read by the depth test.
// Names not in the reference map.
constexpr RndResourceState kStateShaderResource =
    static_cast<RndResourceState>(0xC0);
constexpr RndResourceState kStateNonPixelShaderResource =
    static_cast<RndResourceState>(0x40);

// The compute stage's bit in a stage mask.
constexpr unsigned int kComputeStageMask = 1U << kShaderProgramCompute;
constexpr unsigned int kAllStagesMask = (1U << kNumShaderProgramTypes) - 1;

RndResourceBarrier Transition(
    RndShaderResource* resource,
    RndResourceState before,
    RndResourceState after) {
    RndResourceBarrier barrier;
    barrier.mResource = resource;
    barrier.mSubresource = ~0UL;
    barrier.mBefore = before;
    barrier.mAfter = after;
    return barrier;
}

RndResourceBarrier UnorderedAccess(RndShaderResource* resource) {
    RndResourceBarrier barrier;
    barrier.mType = RndResourceBarrierType::kUnorderedAccess;
    barrier.mResource = resource;
    return barrier;
}

// Unbinds the source textures of the stages and forgets their slot
// limits, as the binary inlines it.
void DeselectSourceTextures(RndContext& ctx, unsigned int stages) {
    ctx._DeselectAllSourceTexturesImpl(stages);
    for (unsigned int stage = 0; stage < kNumShaderProgramTypes; ++stage) {
        if ((stages & (1U << stage)) != 0) {
            ctx.mOutputSlotLimits[stage] = 0;
        }
    }
}

// The debug views (user shading modes 6 to 17) go to the debug shading
// mode; the others shade normally.
void SelectLightingShadingMode(RndContext& ctx, int mode) {
    if (static_cast<unsigned int>(mode - 6) <= 11) {
        ctx.SetShaderDebugMode(
            static_cast<RndUserShadingMode>(mode), kShadingModeStandard);
    } else {
        ctx.SetShadingMode(kShadingModeStandard);
    }
}

RndBufferCollection::FrameIntervalBuffers& Frame(
    RndBufferCollection& buffers) {
    return buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
}

// The draw target's light buffers: the collection's ping-pong pair, or
// the frame interval's copy when no scene context is active.
RndTextureBase* SrcLightAccum(
    RndBufferCollection& buffers,
    const RndSceneDrawTarget& target) {
    return buffers.mActiveSceneContext != 0
        ? buffers.mLightAccum[target.mSrcLightAccum]
        : Frame(buffers).mPartialLightAccum;
}

RndTextureBase* DstLightAccum(
    RndBufferCollection& buffers,
    const RndSceneDrawTarget& target) {
    return buffers.mActiveSceneContext != 0
        ? buffers.mLightAccum[target.mDstLightAccum]
        : Frame(buffers).mPartialLightAccum;
}

void SwapLightAccum(RndSceneDrawTarget& target) {
    const unsigned long src = target.mSrcLightAccum;
    target.mSrcLightAccum = target.mDstLightAccum;
    target.mDstLightAccum = src;
    target.mResult = nullptr;
}

// No probe: the state indices are -1.
RndLightProbeParams NoProbe() {
    RndLightProbeParams probe;
    probe.mProbe = nullptr;
    probe.mStateIndices[0] = static_cast<unsigned long>(-1);
    probe.mStateIndices[1] = static_cast<unsigned long>(-1);
    probe.mStateBlend = 0.0F;
    return probe;
}

// The name of a GPU statistic, a symbol built on first use.
const char* StatName(Symbol& name, const char* text) {
    if (name == Symbol()) {
        name = Symbol(text);
    }
    return name.Str();
}

// The cookie texture arrays' requested usage and the flags of the arrays
// for rendered cookies. Names not in the reference map.
constexpr int kCookieTextureUsage = 9;
constexpr unsigned int kRenderedCookieFlags = 0xA;

// 1 for a cookie that is a render target, 0 for one loaded from a file.
int IsRenderedCookie(const RndTextureBase& cookie) {
    return (cookie.mBaseDesc.mFormat.mFlags & kPixelFormatRenderTarget) != 0
        ? 1
        : 0;
}

// The mip level of square pixels that is `size` wide, or null when the
// pixels are not square or are smaller.
const RndPixelData* FindCookieMip(const RndPixelData& pixels, int size) {
    if (pixels.mSize.x != pixels.mSize.y || pixels.mSize.x < size) {
        return nullptr;
    }
    const RndPixelData* level = &pixels;
    while (level != nullptr && level->mSize.x > size) {
        level = level->mMip;
    }
    return level;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x48ECC0).
Symbol RndLightMgrCom::sId("LightMgr");
Symbol RndLightMgrCom::sClassName("LightMgr");
PropRegistry RndLightMgrCom::sPropRegistry;
ComMetaData RndLightMgrCom::sMetaData;

// Reconstructed from eboot.elf at 0x480280. The members from mCulling are
// the map's RuntimeData, whose constructor is at 0x4803B0.
RndLightMgrCom::RndLightMgrCom()
    : mDefaultEnvironment(gNullObjectId),
      mAmbientOcclusion(gNullObjectId),
      mMasterIntensityMult(1.0F),
      mTonemapping(0),
      mTonemappingOperator(0),
      mTonemappingExposure(1.0F),
      mTonemappingPower(1.0F),
      mMaterialSmoothnessAdjustment(2.0F),
      mCookieTextureSize(256),
      mCurStateA(RndLightProbeCom::GetDefaultStateName()),
      mCurStateB(RndLightProbeCom::GetDefaultStateName()),
      mCurStateBlend(0.0F),
      mCaptureState(RndLightProbeCom::GetDefaultStateName()),
      mCaptureNumBounces(1),
      mActiveFilter(3),
      mEntityFilter(3),
      mShadowFilter(3),
      mVolumetricFilter(3),
      mCulling(false),
      mLightsCulled(false),
      mSpotShadowConfigIndex(-1),
      mLightRemovalCount(0),
      mDefaultEnvironDirty(false),
      mCookieTexArraysDirty(false),
      mCookieTexArrays{},
      mProbeTexArraysDirty(false),
      mProbeTexArrays{},
      mSpotShadowDepthTexArrayDirty(false),
      mSpotShadowDepthTexArray(nullptr),
      mNumShadowContributions(0),
      mNumSpotShadowDepthLayers(0),
      mIsolatedLight(static_cast<std::int32_t>(gNullObjectId.mId)),
      mIsolatedLightCom(nullptr),
      mIsolatedProbe(static_cast<std::int32_t>(gNullObjectId.mId)),
      mIsolatedProbeCom(nullptr),
      mCurProbeStateAIndex(static_cast<std::uint64_t>(-1)),
      mCurProbeStateBIndex(static_cast<std::uint64_t>(-1)),
      mUpgradeProbeStates(false),
      mAmbientOcclusionGenerated(false),
      mLightBuffers{},
      mLightProbeBuffer(nullptr),
      mSliceZeroLightIdBuffer(nullptr) {
    mLeavingPlayModeSink.mSink = nullptr;
    mLeavingPlayModeSink.mSource = nullptr;
    mLeavingRecordModeSink.mSink = nullptr;
    mLeavingRecordModeSink.mSource = nullptr;
}

// Inlined into _Imprint at 0x48CB12. The property arrays copy their
// elements (PropArrayBase::_Copy, 0xBA70); the run-time state is
// constructed empty (RuntimeData's constructor, 0x4803B0).
RndLightMgrCom::RndLightMgrCom(const RndLightMgrCom& other)
    : Component(other),
      mDefaultEnvironment(other.mDefaultEnvironment),
      mAmbientOcclusion(other.mAmbientOcclusion),
      mMasterIntensityMult(other.mMasterIntensityMult),
      mTonemapping(other.mTonemapping),
      mTonemappingOperator(other.mTonemappingOperator),
      mTonemappingExposure(other.mTonemappingExposure),
      mTonemappingPower(other.mTonemappingPower),
      mMaterialSmoothnessAdjustment(other.mMaterialSmoothnessAdjustment),
      mCookieTextureSize(other.mCookieTextureSize),
      mCurStateA(other.mCurStateA),
      mCurStateB(other.mCurStateB),
      mCurStateBlend(other.mCurStateBlend),
      mCaptureState(other.mCaptureState),
      mCaptureNumBounces(other.mCaptureNumBounces),
      mActiveFilter(other.mActiveFilter),
      mEntityFilter(other.mEntityFilter),
      mShadowFilter(other.mShadowFilter),
      mVolumetricFilter(other.mVolumetricFilter),
      mCulling(false),
      mLightsCulled(false),
      mSpotShadowConfigIndex(-1),
      mLightRemovalCount(0),
      mDefaultEnvironDirty(false),
      mCookieTexArraysDirty(false),
      mCookieTexArrays{},
      mProbeTexArraysDirty(false),
      mProbeTexArrays{},
      mSpotShadowDepthTexArrayDirty(false),
      mSpotShadowDepthTexArray(nullptr),
      mNumShadowContributions(0),
      mNumSpotShadowDepthLayers(0),
      mIsolatedLight(static_cast<std::int32_t>(gNullObjectId.mId)),
      mIsolatedLightCom(nullptr),
      mIsolatedProbe(static_cast<std::int32_t>(gNullObjectId.mId)),
      mIsolatedProbeCom(nullptr),
      mCurProbeStateAIndex(static_cast<std::uint64_t>(-1)),
      mCurProbeStateBIndex(static_cast<std::uint64_t>(-1)),
      mUpgradeProbeStates(false),
      mAmbientOcclusionGenerated(false),
      mLightBuffers{},
      mLightProbeBuffer(nullptr),
      mSliceZeroLightIdBuffer(nullptr) {
    mProbeStates._Copy(other.mProbeStates);
    mQualitySettings._Copy(other.mQualitySettings);
    mLeavingPlayModeSink.mSink = nullptr;
    mLeavingPlayModeSink.mSource = nullptr;
    mLeavingRecordModeSink.mSink = nullptr;
    mLeavingRecordModeSink.mSource = nullptr;
}

// Reconstructed from eboot.elf at 0x480AD0. A linked subscription leaves
// its source before the link unlinks itself (RuntimeData's destructor,
// 0x48CD50).
RndLightMgrCom::~RndLightMgrCom() {
    SafeDelete(mCookieTexArrays[0]);
    SafeDelete(mCookieTexArrays[2]);
    SafeDelete(mCookieTexArrays[1]);
    SafeDelete(mCookieTexArrays[3]);
    SafeDelete(mProbeTexArrays[0]);
    SafeDelete(mProbeTexArrays[1]);
    SafeDelete(mSpotShadowDepthTexArray);
    for (RndTiledLightsComputeBuffer& buffer : mLightBuffers) {
        SafeDelete(buffer.mBuffer);
    }
    SafeDelete(mLightProbeBuffer);
    SafeDelete(mSliceZeroLightIdBuffer);
    for (MsgSource::EventSinkElem* sink : {&mLeavingRecordModeSink, &mLeavingPlayModeSink}) {
        LinkedList::Node& link = sink->mLink;
        if (link.mNext != &link && link.mPrev != &link) {
            sink->mSource->RemoveSink(sink);
        }
    }
}

// Reconstructed from eboot.elf at 0x480F10. The cookies are kept per
// texture type and per render-target flag, sorted by texture, and keep
// their slice once placed. Only cookies loaded from files get pixels; a
// rendered cookie's slice is recorded without them. The 2D cookies link
// to their array slice.
void RndLightMgrCom::_SyncCookieTexArrays() {
    if (!mCookieTexArraysDirty) {
        return;
    }
    if (!gRndDevice->mSettings->mUseTiledLighting) {
        mCookieTexArraysDirty = false;
        return;
    }

    unsigned long counts[8][2] = {};
    for (RndLightCom* light : mLights) {
        RndTextureBase* cookie = light->_GetCookieTextureImpl();
        if (cookie != nullptr) {
            ++counts[cookie->_GetTypeImpl()][IsRenderedCookie(*cookie)];
        }
    }
    for (int type = 0; type < 8; ++type) {
        mCookieArrayElems[type][0].reserve(counts[type][0]);
        mCookieArrayElems[type][1].reserve(counts[type][1]);
    }

    for (RndLightCom* light : mLights) {
        RndTextureBase* cookie = light->_GetCookieTextureImpl();
        if (cookie == nullptr) {
            light->mRuntime.mTiledCookieIndex = static_cast<unsigned long>(-1);
            light->mRuntime.mCookieTexArrayIndex = -1;
            continue;
        }
        const int rendered = IsRenderedCookie(*cookie);
        eastl::vector<CookieArrayElemInfo>& elems =
            mCookieArrayElems[cookie->_GetTypeImpl()][rendered];
        CookieArrayElemInfo* position = elems.begin();
        for (unsigned long count = elems.size(); count > 0;) {
            const unsigned long half = count / 2;
            if (position[half].mTexture < cookie) {
                position += half + 1;
                count -= half + 1;
            } else {
                count = half;
            }
        }
        unsigned long slice;
        if (position != elems.end() && position->mTexture == cookie) {
            slice = position->mSlice;
        } else {
            const CookieArrayElemInfo elem = {cookie, elems.size()};
            elems.insert(position, elem);
            slice = elem.mSlice;
        }
        light->mRuntime.mTiledCookieIndex = slice;
        light->mRuntime.mCookieTexArrayIndex = rendered;
    }

    // Which slices were filled, per texture type and flag. The binary
    // keeps eastl bitvectors (0x117FBC0) on the temporary heap.
    eastl::vector<bool> filled[8][2];
    unsigned int savedTemp;
    MemPushTemp(savedTemp, true, true);
    for (int type = 0; type < 8; ++type) {
        for (int flag = 0; flag < 2; ++flag) {
            filled[type][flag].clear();
            filled[type][flag].resize(mCookieArrayElems[type][flag].size());
        }
    }
    MemPopTemp(savedTemp);

    // [0] for cookies loaded from files, [1] for rendered ones.
    RndTextureArray2D::Description descs2D[2];
    RndTextureArrayCube::Description descsCube[2];
    const char* const names[2] = {"2D Light Cookies", "Cube Light Cookies"};
    for (int flag = 0; flag < 2; ++flag) {
        RndTextureBase::Description* descs[2] = {
            &descs2D[flag], &descsCube[flag]};
        const int types[2] = {
            RndTextureBase::kTexture2D, RndTextureBase::kTextureCube};
        for (int kind = 0; kind < 2; ++kind) {
            RndTextureBase::Description& desc = *descs[kind];
            desc.mName = names[kind];
            desc.mRequestedFormat = RndPixelFormat{};
            desc.mRequestedFormat.mUsage = kCookieTextureUsage;
            desc.mRequestedFormat.mFlags = flag != 0 ? kRenderedCookieFlags : 0;
            desc.ResolveFormat(types[kind], -1);
            desc.mDataFormat = RndResolveDataFormat(desc.mFormat, -1, -1);
        }
    }

    MemPushTemp(savedTemp, true, true);
    const int size = static_cast<int>(mCookieTextureSize);
    const Vector3i extent{size, size, 1};
    for (int flag = 0; flag < 2; ++flag) {
        RndTextureArray2D::Description& desc2D = descs2D[flag];
        desc2D.mPixels.resize(mCookieArrayElems[RndTextureBase::kTexture2D][flag].size());
        for (RndPixelData& pixels : desc2D.mPixels) {
            if ((desc2D.mFormat.mFlags & kPixelFormatRenderTarget) != 0) {
                pixels.CreateEmpty(extent, desc2D.mDataFormat);
            } else {
                pixels.Create(extent, desc2D.mDataFormat, nullptr);
                pixels.CreateMips();
            }
        }
        RndTextureArrayCube::Description& descCube = descsCube[flag];
        descCube.mCubes.resize(mCookieArrayElems[RndTextureBase::kTextureCube][flag].size());
        for (RndPixelDataCube& cube : descCube.mCubes) {
            if ((descCube.mFormat.mFlags & kPixelFormatRenderTarget) != 0) {
                cube.CreateEmpty(size, descCube.mDataFormat);
            } else {
                cube.CreateUninitialized(size, descCube.mDataFormat);
                cube.CreateMips();
            }
        }
    }
    MemPopTemp(savedTemp);

    for (RndLightCom* light : mLights) {
        RndTextureBase* cookie = light->_GetCookieTextureImpl();
        if (cookie == nullptr) {
            continue;
        }
        const int type = cookie->_GetTypeImpl();
        const unsigned long slice = light->mRuntime.mTiledCookieIndex;
        const int rendered = IsRenderedCookie(*cookie);
        eastl::vector<bool>& done = filled[type][rendered];
        if (done[slice]) {
            continue;
        }
        done[slice] = true;
        if (rendered != 0) {
            continue;
        }
        const char* name = cookie->mBaseDesc.mName;
        if (type == RndTextureBase::kTexture2D) {
            RndPixelData scratch;
            MemPushTemp(savedTemp, true, true);
            const RndPixelData* pixels = _GetCookieArrayPixels(
                name,
                descs2D[0].mDataFormat,
                static_cast<RndTexture2D*>(cookie)->mPixels,
                scratch);
            MemPopTemp(savedTemp);
            descs2D[0].mPixels[slice].CopyFrom(*pixels);
        } else {
            RndPixelDataCube scratch;
            MemPushTemp(savedTemp, true, true);
            const RndPixelDataCube& cube = _GetCookieArrayPixels(
                name,
                descsCube[0].mDataFormat,
                static_cast<RndTextureCube*>(cookie)->mCube,
                scratch);
            MemPopTemp(savedTemp);
            descsCube[0].mCubes[slice].CopyFrom(cube);
        }
    }

    SafeDelete(mCookieTexArrays[0]);
    if (!descs2D[0].mPixels.empty()) {
        mCookieTexArrays[0] = RndTextureArray2D::New(descs2D[0]);
    }
    SafeDelete(mCookieTexArrays[2]);
    if (!descsCube[0].mCubes.empty()) {
        mCookieTexArrays[2] = RndTextureArrayCube::New(descsCube[0], nullptr);
    }
    SafeDelete(mCookieTexArrays[1]);
    if (!descs2D[1].mPixels.empty()) {
        mCookieTexArrays[1] = RndTextureArray2D::New(descs2D[1]);
    }
    SafeDelete(mCookieTexArrays[3]);
    if (!descsCube[1].mCubes.empty()) {
        mCookieTexArrays[3] = RndTextureArrayCube::New(descsCube[1], nullptr);
    }

    for (RndLightCom* light : mLights) {
        RndTextureBase* cookie = light->_GetCookieTextureImpl();
        if (cookie != nullptr && cookie->_GetTypeImpl() == RndTextureBase::kTexture2D) {
            static_cast<RndTexture2D*>(cookie)->SetLinkedTexture(
                mCookieTexArrays[IsRenderedCookie(*cookie)],
                static_cast<long>(light->mRuntime.mTiledCookieIndex));
        }
    }
    mCookieTexArraysDirty = false;
}

// Reconstructed from eboot.elf at 0x482210.
void RndLightMgrCom::AddEnviron(RndLightEnvironCom* environ) {
    const GameObjectId id = environ->mObject->mId;
    if (mEnvironments.size() == 0) {
        mEnvironments._Insert(0, &gNullObjectId);
    }
    unsigned long first;
    unsigned long last;
    if (id.mId == mDefaultEnvironment.mId) {
        mEnvironments[0] = id;
        first = 0;
        last = 1;
    } else {
        first = mEnvironments.size();
        mEnvironments._Insert(first, &id);
        last = mEnvironments.size();
    }
    _ReindexEnvirons(first, last);
}

// Reconstructed from eboot.elf at 0x4822C0. The objects must hold a
// RndLightEnvironCom; the indexing of a seventh environment only builds
// the error name of a warning that is compiled out.
void RndLightMgrCom::_ReindexEnvirons(unsigned long first, unsigned long last) {
    mEnvironComs.resize(mEnvironments.size());
    const Entity* entity = mObject->mEntity;
    for (unsigned long i = first; i < last; ++i) {
        const GameObjectId id = mEnvironments[i];
        GameObject* object = id.mId != gNullObjectId.mId ? entity->GetObject(id) : nullptr;
        if (object == nullptr) {
            mEnvironComs[i] = nullptr;
            continue;
        }
        RndLightEnvironCom* environ = object->GetCom<RndLightEnvironCom>();
        mEnvironComs[i] = environ;
        environ->SetEnvironIndex(RndLightEnvironCom::kNoEnvironIndex);
    }
    for (unsigned long i = first; i < last; ++i) {
        const GameObjectId id = mEnvironments[i];
        GameObject* object = id.mId != gNullObjectId.mId ? entity->GetObject(id) : nullptr;
        if (object == nullptr) {
            continue;
        }
        RndLightEnvironCom* environ = object->GetCom<RndLightEnvironCom>();
        if (i >= kMaxIndexedEnvirons) {
            static_cast<void>(object->MakeErrorName());
            environ->SetEnvironIndex(RndLightEnvironCom::kNoEnvironIndex);
        } else {
            environ->SetEnvironIndex(static_cast<int>(i));
        }
    }
}

// Reconstructed from eboot.elf at 0x4824D0. The default environment keeps
// its slot.
void RndLightMgrCom::RemoveEnviron(RndLightEnvironCom* environ) {
    const GameObjectId id = environ->mObject->mId;
    const unsigned long index = Find(mEnvironments, id);
    if (id.mId == mDefaultEnvironment.mId) {
        mEnvironments[0] = gNullObjectId;
        mEnvironComs[0] = nullptr;
    } else {
        RemoveAt(mEnvironments, index);
        _ReindexEnvirons(index, mEnvironments.size());
    }
    environ->SetEnvironIndex(RndLightEnvironCom::kNoEnvironIndex);
}

// Reconstructed from eboot.elf at 0x482770.
RndLightEnvironCom* RndLightMgrCom::GetDefaultEnviron() const {
    if (mDefaultEnvironment.mId == gNullObjectId.mId) {
        return nullptr;
    }
    const GameObject* object = mObject->mEntity->GetObject(mDefaultEnvironment);
    return object != nullptr ? object->GetCom<RndLightEnvironCom>() : nullptr;
}

// Reconstructed from eboot.elf at 0x482800.
void RndLightMgrCom::SetDefaultEnvironId(GameObjectId id) {
    if (mDefaultEnvironment.mId != id.mId) {
        mDefaultEnvironment = id;
        mDefaultEnvironDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x482810.
void RndLightMgrCom::AddLight(RndLightCom* light) {
    mLights.push_back(light);
    light->mRuntime.mMasterIntensityMult = mMasterIntensityMult;
    if (light->_GetCookieTextureImpl() != nullptr) {
        mCookieTexArraysDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x482920.
void RndLightMgrCom::RemoveLight(RndLightCom* light) {
    ++mLightRemovalCount;
    EraseElement(mLights, light);
    if (mIsolatedLightCom == light) {
        mIsolatedLight = static_cast<std::int32_t>(gNullObjectId.mId);
        mIsolatedLightCom = nullptr;
    }
    light->mRuntime.mTiledCookieIndex = static_cast<unsigned long>(-1);
    light->mRuntime.mCookieTexArrayIndex = -1;
}

// Reconstructed from eboot.elf at 0x4829F0.
void RndLightMgrCom::AddProbe(RndLightProbeCom* probe) {
    mProbes.push_back(probe);
    const GameObjectId id = probe->mObject->mId;
    mProbeIds._Insert(mProbeIds.size(), &id);
    mProbeTexArraysDirty = true;
}

// Reconstructed from eboot.elf at 0x482B30.
void RndLightMgrCom::RemoveProbe(RndLightProbeCom* probe) {
    ++mLightRemovalCount;
    EraseElement(mProbes, probe);
    RemoveAt(mProbeIds, Find(mProbeIds, probe->mObject->mId));
    if (mIsolatedProbeCom == probe) {
        mIsolatedProbe = static_cast<std::int32_t>(gNullObjectId.mId);
        mIsolatedProbeCom = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x482C80.
void RndLightMgrCom::SetIsolatedProbe(RndLightProbeCom* probe) {
    mIsolatedProbe = static_cast<std::int32_t>(probe != nullptr ? probe->mObject->mId.mId : gNullObjectId.mId);
    mIsolatedProbeCom = probe;
}

// Reconstructed from eboot.elf at 0x482CB0. Only lights that add light cast
// shadows.
void RndLightMgrCom::CullLights(
    unsigned char* sceneDrawId,
    unsigned int sceneDrawIndex,
    const RndCameraContext& camera,
    const RndSceneDrawParams& params) {
    mCulling = true;
    mLightsCulled = true;
    _FrustumCullLights(sceneDrawId, sceneDrawIndex, camera, params);
    mNumShadowContributions = 0;
    mNumSpotShadowDepthLayers = 0;
    const RndConfig& config = *gRndDevice->mSettings;
    for (auto& lists : mCullResults.mLights) {
        for (RndLightCom* light : lists[0]) {
            const bool castsShadows = light->_CastsShadowsImpl(config.mQualityLevel);
            if (light->_AcquireDeferredShadowContributionResourcesImpl(castsShadows && params.mDrawShadows, *this)) {
                mCullResults.mShadowLights.push_back(light);
            }
        }
    }
    mCulling = false;
}

// Reconstructed from eboot.elf at 0x482EB0. A light is kept when it is
// visible, passes the scene's show and hide flags, lights an environment
// or the volumetrics, and its frustum test passes. Without tiled lighting
// the environments receive their culled lights. A camera whose target mode
// is 1 also culls against the first of the light tiles' depth slices.
void RndLightMgrCom::_FrustumCullLights(
    unsigned char* sceneDrawId,
    unsigned int sceneDrawIndex,
    const RndCameraContext& camera,
    const RndSceneDrawParams& params) {
    static_cast<void>(sceneDrawId);
    static_cast<void>(sceneDrawIndex);
    const RndLightCullPlanes planes{camera.mDerivedCache.mData, camera.mDerivedCache.mSize};
    CullResults& results = mCullResults;
    Clear(results);
    for (auto& lists : results.mLights) {
        lists[0].reserve(mLights.size());
        lists[1].reserve(mLights.size());
    }
    results.mProbes.reserve(mProbes.size());

    const RndConfig& config = *gRndDevice->mSettings;
    const unsigned int mode = static_cast<unsigned int>(params.mShadingMode);
    if (ShadingModeKeepsLights(mode)) {
        const RndShowHideContext& showHide = params.mShowHide;
        for (RndLightCom* light : mLights) {
            if (!light->mRuntime.mVisible) {
                continue;
            }
            const unsigned int flags = light->mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags;
            if (showHide.mShowFlags != 0 && (flags & showHide.mShowFlags) == 0) {
                continue;
            }
            if ((flags & showHide.mHideFlags) != 0) {
                continue;
            }
            if (light->mRuntime.mEnvironBits == 0 && (!light->mVolumetric || !config.mVolumetricScatteringEnabled)) {
                continue;
            }
            if (light->_FrustumExcludesImpl(camera.mFrustum, planes)) {
                continue;
            }
            const int type = light->_GetTypeImpl();
            results.mLights[type][light->mIlluminationType == 3 ? 1 : 0].push_back(light);
        }
    }
    if (ShadingModeKeepsProbes(mode)) {
        for (RndLightProbeCom* probe : mProbes) {
            if (probe->mRuntime.mVisible && !probe->_FrustumExcludes(camera.mFrustum, planes)) {
                results.mProbes.push_back(probe);
            }
        }
    }

    if (!config.mUseTiledLighting) {
        const unsigned long numEnvirons = mEnvironComs.size();
        for (RndLightEnvironCom* environ : mEnvironComs) {
            if (environ != nullptr) {
                environ->ClearCulledLights();
            }
        }
        for (auto& lists : results.mLights) {
            for (auto& list : lists) {
                for (RndLightCom* light : list) {
                    const unsigned int bits = light->mRuntime.mEnvironBits;
                    const int type = light->_GetTypeImpl();
                    const int illuminationType = light->mIlluminationType;
                    for (unsigned long i = 0; i < numEnvirons; ++i) {
                        if (((bits >> i) & 1) != 0) {
                            mEnvironComs[i]->AddCulledLight(light, type, illuminationType);
                        }
                    }
                }
            }
        }
    }

    if (camera.mTargetMode != 1) {
        return;
    }
    const float slice = 1.0F / static_cast<float>(static_cast<std::uint64_t>(config.mLightTileDepthSlices));
    const Vector3* corners = camera.mFrustum.mCorners.mData;
    Vector3 sliceCorners[8];
    for (int i = 0; i < 4; ++i) {
        sliceCorners[i] = corners[i];
        sliceCorners[i + 4] = slice == 0.0F ? corners[i] : LerpCorner(corners[i], corners[i + 4], slice);
    }
    Frustum sliceFrustum;
    sliceFrustum.SetCorners(sliceCorners);

    CullResults& sliceResults = mSliceZeroCullResults;
    Clear(sliceResults);
    sliceResults.ReserveFor(results);
    const RndLightCullPlanes noPlanes{nullptr, 0};
    for (int type = 0; type < 3; ++type) {
        for (int list = 0; list < 2; ++list) {
            for (RndLightCom* light : results.mLights[type][list]) {
                if (!light->_FrustumExcludesImpl(sliceFrustum, noPlanes)) {
                    sliceResults.mLights[type][list].push_back(light);
                }
            }
        }
    }
    for (RndLightProbeCom* probe : results.mProbes) {
        if (!probe->_FrustumExcludes(sliceFrustum, noPlanes)) {
            sliceResults.mProbes.push_back(probe);
        }
    }
}

// Reconstructed from eboot.elf at 0x4840D0.
void RndLightMgrCom::_CullTiledLights(
    RndContext& ctx,
    const RndCameraContext& camera,
    const RndSceneDrawParams& params,
    RndBufferCollection& buffers,
    RndBufferCollection* rightEyeBuffers) {
    if (!gRndDevice->mSettings->mUseTiledLighting) {
        return;
    }
    _FillTiledLightBuffers(ctx, camera);
    if (camera.mTargetMode == 1) {
        _FillSliceZeroLightIds(ctx);
    }
    _DispatchTiledLightCull(ctx, camera, params, buffers, rightEyeBuffers);
}

// Reconstructed from eboot.elf at 0x484160.
void RndLightMgrCom::_FillTiledLightBuffers(RndContext& ctx, const RndCameraContext& camera) {
    const RndConfig& config = *gRndDevice->mSettings;
    for (RndTiledLightsComputeBuffer& lights : mLightBuffers) {
        lights.mBuffer->mStagingSize = 0;
    }
    for (unsigned long type = 0; type < kNumTiledLightsBufferTypes; ++type) {
        RndTiledLightsComputeBuffer& lights = mLightBuffers[type];
        RndComputeBuffer& buffer = *lights.mBuffer;
        unsigned long* counts[2] = {&lights.mNumPosLights, &lights.mNumNegLights};
        for (int list = 0; list < 2; ++list) {
            *counts[list] = 0;
            for (RndLightCom* light : mCullResults.mLights[type][list]) {
                const unsigned int index = static_cast<unsigned int>(buffer.mStagingSize / buffer.mDesc.mElementSize);
                if (index >= buffer.mDesc.mNumElements) {
                    static_cast<void>(RndLightUtl::GetLightTypeName(static_cast<RndLightType>(type)));
                    break;
                }
                light->mRuntime.mLightBufferIndex = static_cast<int>(index);
                light->_AddToComputeBufferImpl(config.mQualityLevel, camera, buffer);
                ++*counts[list];
            }
        }
    }
    for (RndTiledLightsComputeBuffer& lights : mLightBuffers) {
        lights.mBuffer->_SyncDynamicImpl(ctx);
    }

    RndComputeBuffer& probes = *mLightProbeBuffer;
    probes.mStagingSize = 0;
    unsigned int index = 0;
    for (RndLightProbeCom* probe : mCullResults.mProbes) {
        if (index >= probes.mDesc.mNumElements) {
            break;
        }
        probe->mRuntime.mComputeBufferIndex = static_cast<int>(index);
        probe->AddToComputeBuffer(camera, mCurProbeStateAIndex, mCurProbeStateBIndex, mCurStateBlend, probes);
        index = static_cast<unsigned int>(probes.mStagingSize / probes.mDesc.mElementSize);
    }
    probes._SyncDynamicImpl(ctx);
}

// Reconstructed from eboot.elf at 0x484410. The probes come first, then the
// lights that add light and those that subtract it, by type.
void RndLightMgrCom::_FillSliceZeroLightIds(RndContext& ctx) {
    RndComputeBuffer& buffer = *mSliceZeroLightIdBuffer;
    buffer.mStagingSize = 0;
    auto append = [&buffer](int id) {
        *reinterpret_cast<int*>(static_cast<char*>(buffer.mStagingData) + buffer.mStagingSize) = id;
        buffer.mStagingSize += sizeof(int);
    };
    for (RndLightProbeCom* probe : mSliceZeroCullResults.mProbes) {
        append(probe->mRuntime.mComputeBufferIndex);
    }
    for (int list = 0; list < 2; ++list) {
        for (auto& lists : mSliceZeroCullResults.mLights) {
            for (RndLightCom* light : lists[list]) {
                append(light->mRuntime.mLightBufferIndex);
            }
        }
    }
    buffer._SyncDynamicImpl(ctx);
}

// Reconstructed from eboot.elf at 0x484620. The tile lists are written as
// unordered-access buffers and handed back to the shaders.
void RndLightMgrCom::_DispatchTiledLightCull(
    RndContext& ctx,
    const RndCameraContext& camera,
    const RndSceneDrawParams& params,
    RndBufferCollection& buffers,
    RndBufferCollection* rightEyeBuffers) {
    static Symbol sLighting;
    RndScopedGpuStatBlock stat(ctx, StatName(sLighting, "Lighting"));
    RndLightGlobals& lighting = gRndDevice->mLighting;

    RndCShaderClearBuffer::Params clear;
    clear.mBuffer = lighting.mTiledLightIdsCount;
    clear.mNumericType = kShaderNumericUInt;
    gRndDevice->mShaderMgr.mClearBufferCShader->Dispatch(ctx, clear);

    RndBufferCollection::FrameIntervalBuffers& frame = Frame(buffers);
    FixedVector<RndResourceBarrier, 4> barriers;
    barriers.push_back(Transition(
        frame.mTiledLightIds[0],
        kStateShaderResource,
        RndResourceState::kUnorderedAccess));
    barriers.push_back(Transition(
        frame.mTiledLightIds[1],
        kStateShaderResource,
        RndResourceState::kUnorderedAccess));
    barriers.push_back(Transition(
        frame.mTiledLightIdRanges,
        kStateShaderResource,
        RndResourceState::kUnorderedAccess));
    barriers.push_back(UnorderedAccess(lighting.mTiledLightIdsCount));
    ctx._ResourceBarrierImpl(barriers.size(), barriers.mData);

    RndCShaderTiledLightsCull::Params cull{};
    cull.mCamera = &camera;
    cull.mBuffers = &buffers;
    cull.mRightEyeBuffers = rightEyeBuffers;
    cull.mLightBuffers = mLightBuffers;
    cull.mLightProbes = mLightProbeBuffer;
    cull.mUseSceneMask = params.mDrawSceneMask;
    lighting.mTiledLightsCullShader->Dispatch(ctx, cull);

    barriers.resize(3);
    for (RndResourceBarrier& barrier : barriers) {
        barrier.mBefore = RndResourceState::kUnorderedAccess;
        barrier.mAfter = kStateShaderResource;
    }
    ctx._ResourceBarrierImpl(barriers.size(), barriers.mData);
    DeselectSourceTextures(ctx, kAllStagesMask);
}

// Reconstructed from eboot.elf at 0x484980. A stereo eye's camera (target
// mode 1) writes the collection's stereo tile lists.
void RndLightMgrCom::_CullTiledLightsStereo(
    RndContext& ctx,
    const RndSceneDrawParams& params,
    const RndCameraContext& stereoCamera,
    const RndCameraContext& eyeCamera,
    RndBufferCollection& source,
    RndBufferCollection& target) {
    static Symbol sLighting;
    RndScopedGpuStatBlock stat(ctx, StatName(sLighting, "Lighting"));
    if (eyeCamera.mTargetMode != 3) {
        _FillTiledLightBuffers(ctx, eyeCamera);
    }
    RndLightGlobals& lighting = gRndDevice->mLighting;

    RndCShaderClearBuffer::Params clear;
    clear.mBuffer = lighting.mTiledLightIdsCount;
    clear.mNumericType = kShaderNumericUInt;
    gRndDevice->mShaderMgr.mClearBufferCShader->Dispatch(ctx, clear);

    RndBufferCollection::FrameIntervalBuffers& frame = Frame(target);
    const bool stereo = eyeCamera.mTargetMode == 1;
    RndShaderResource* const lists[3] = {
        stereo ? frame.mStereoTiledLightIds[0] : frame.mTiledLightIds[0],
        stereo ? frame.mStereoTiledLightIds[1] : frame.mTiledLightIds[1],
        stereo ? frame.mStereoTiledLightIdRanges : frame.mTiledLightIdRanges,
    };
    FixedVector<RndResourceBarrier, 3> barriers;
    for (RndShaderResource* list : lists) {
        barriers.push_back(Transition(
            list, kStateShaderResource, RndResourceState::kUnorderedAccess));
    }
    ctx._ResourceBarrierImpl(barriers.size(), barriers.mData);

    const CullResults& slice = mSliceZeroCullResults;
    RndCShaderTiledLightsCull::Params cull{};
    cull.mCamera = &eyeCamera;
    cull.mBuffers = &target;
    cull.mRightEyeBuffers = nullptr;
    cull.mLightBuffers = mLightBuffers;
    cull.mLightProbes = mLightProbeBuffer;
    cull.mSliceZeroLightIds = mSliceZeroLightIdBuffer;
    for (int type = 0; type < 2; ++type) {
        for (int list = 0; list < 2; ++list) {
            cull.mNumSliceZeroLights[type][list] = slice.mLights[type][list].size();
        }
    }
    cull.mNumSliceZeroProbes = slice.mProbes.size();
    cull.mUseSceneMask = params.mDrawSceneMask;
    cull.mOnlySliceZero = true;
    lighting.mTiledLightsCullShader->Dispatch(ctx, cull);

    for (RndResourceBarrier& barrier : barriers) {
        barrier.mBefore = RndResourceState::kUnorderedAccess;
        barrier.mAfter = kStateShaderResource;
    }
    ctx._ResourceBarrierImpl(barriers.size(), barriers.mData);

    RndCShaderTiledLightsStereoToMono::Params split;
    split.mBothEyesCamera = &stereoCamera;
    split.mCamera = &eyeCamera;
    split.mBothEyesBuffers = &source;
    split.mBuffers = &target;
    split.mUseSceneMask = params.mDrawSceneMask;
    lighting.mTiledLightsStereoToMonoShader->Dispatch(ctx, split);
}

// Reconstructed from eboot.elf at 0x484D80.
void RndLightMgrCom::StoreCullResults(
    RndScenePartialFramerateData& data,
    RndSceneCullResults& results) const {
    data.mLightMgrUpdateFlag = mLightsCulled;
    results.mLights = mCullResults;
}

// Reconstructed from eboot.elf at 0x485220.
void RndLightMgrCom::RestoreCullResults(
    RndContext& ctx,
    const RndCameraContext& camera,
    RndScenePartialFramerateData& data,
    RndSceneCullResults& results) {
    mLightsCulled = data.mLightMgrUpdateFlag;
    mCullResults = results.mLights;
    _FillTiledLightBuffers(ctx, camera);
}

// Reconstructed from eboot.elf at 0x485270. Only lights that add light
// and cast shadows at the quality level contribute shadows; the scene mask
// limits them when the scene draws one.
void RndLightMgrCom::AccumDeferredLight(
    RndContext& ctx,
    const RndCameraContext& camera,
    const RndSceneDrawParams& params,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    RndTextureBase* srcLightAccum,
    RndFence* fence) {
    const RndConfig& config = *gRndDevice->mSettings;
    {
        static Symbol sShadows;
        RndScopedGpuStatBlock stat(ctx, StatName(sShadows, "Shadows"));
        for (auto& lists : mCullResults.mLights) {
            for (RndLightCom* light : lists[0]) {
                if (light->_CastsShadowsImpl(config.mQualityLevel)) {
                    RndTextureBase* sceneMask = params.mDrawSceneMask
                        ? buffers.mTiledSceneMask[1]
                        : nullptr;
                    light->_GenerateDeferredShadowContributionImpl(
                        ctx, buffers, target, sceneMask, *this);
                }
            }
        }
    }
    static Symbol sLighting;
    RndScopedGpuStatBlock stat(ctx, StatName(sLighting, "Lighting"));
    if (gRndDevice->mSettings->mUseTiledLighting) {
        _AccumTiledDeferredLight(
            ctx, camera, params, buffers, target, srcLightAccum, fence);
    } else {
        _AccumUntiledDeferredLight(
            ctx, params, buffers, target, srcLightAccum);
    }
}

// Reconstructed from eboot.elf at 0x485590. The interpolated pass reads
// the first frame interval's interpolation buffer; the GPU waits on the
// fence before shading. Without a generated AO the default white texture
// stands in for it.
void RndLightMgrCom::_AccumTiledDeferredLight(
    RndContext& ctx,
    const RndCameraContext& camera,
    const RndSceneDrawParams& params,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    RndTextureBase* srcLightAccum,
    RndFence* fence) {
    const RndConfig& config = *gRndDevice->mSettings;
    ctx.SetRenderTargets(nullptr, nullptr);
    SelectLightingShadingMode(ctx, params.mShadingMode);

    RndBufferCollection::FrameIntervalBuffers& frame = Frame(buffers);
    RndTextureBase* const interp =
        buffers.mFrameIntervals.mData[0].mTiledLightInterp;
    FixedVector<RndResourceBarrier, 4> barriers;
    barriers.push_back(Transition(
        frame.mDepthStencil,
        RndResourceState::kDepthWrite,
        kStateNonPixelShaderResource));
    barriers.push_back(Transition(
        DstLightAccum(buffers, target),
        kStateShaderResource,
        RndResourceState::kUnorderedAccess));
    barriers.push_back(Transition(
        srcLightAccum,
        RndResourceState::kRenderTarget,
        kStateShaderResource));
    if (config.mTiledLightInterpolationEnabled) {
        barriers.push_back(Transition(
            interp,
            kStateNonPixelShaderResource,
            RndResourceState::kUnorderedAccess));
    }
    ctx._ResourceBarrierImpl(barriers.size(), barriers.mData);
    if (fence != nullptr) {
        ctx._WaitFenceImpl(*fence);
    }

    RndLightGlobals& lighting = gRndDevice->mLighting;
    RndCShaderTiledLightsApplication::Params apply;
    apply.mCamera = &camera;
    apply.mBuffers = &buffers;
    // The first 41 bytes of the draw target.
    std::memcpy(
        reinterpret_cast<char*>(&apply)
            + offsetof(RndCShaderTiledLightsApplication::Params,
                       mDrawParamsHeader),
        &target,
        41);
    apply.mLightBuffers = mLightBuffers;
    apply.mLightProbes = mLightProbeBuffer;
    apply.mCookies2D = mCookieTexArrays[0];
    apply.mCookies2DRendered = mCookieTexArrays[1];
    apply.mCookiesCube = mCookieTexArrays[2];
    apply.mProbeDiffuseTextures = mProbeTexArrays[0];
    apply.mProbeSpecularTextures = mProbeTexArrays[1];
    apply.mAmbientOcclusion = mAmbientOcclusionGenerated
        ? frame.mAO
        : gRndDevice->mDefaults.mTextures2D[kDefaultTextureWhite];
    apply.mSrcLightAccumBuffer = srcLightAccum;
    apply.mProbeIntensityMult = mMasterIntensityMult;
    lighting.mTiledLightsApplicationShader->Dispatch(ctx, apply);

    if (config.mTiledLightInterpolationEnabled) {
        FixedVector<RndResourceBarrier, 2> interpBarriers;
        interpBarriers.push_back(Transition(
            interp,
            RndResourceState::kUnorderedAccess,
            kStateNonPixelShaderResource));
        interpBarriers.push_back(
            UnorderedAccess(DstLightAccum(buffers, target)));
        ctx._ResourceBarrierImpl(interpBarriers.size(), interpBarriers.mData);
        DeselectSourceTextures(ctx, kComputeStageMask);
        lighting.mTiledLightsInterpolationShader->Dispatch(
            ctx, buffers, target, *srcLightAccum);
    }

    barriers.resize(2);
    barriers[0].mBefore = kStateNonPixelShaderResource;
    barriers[0].mAfter = RndResourceState::kDepthWrite;
    barriers[1].mBefore = RndResourceState::kUnorderedAccess;
    barriers[1].mAfter = RndResourceState::kRenderTarget;
    ctx._ResourceBarrierImpl(barriers.size(), barriers.mData);
    SwapLightAccum(target);
    ctx.SetShadingMode(kShadingModeStandard);
}

// Reconstructed from eboot.elf at 0x4859B0. The point and spot lights draw
// without a probe; the probes are drawn on their own unless the
// directional lights took the single probe.
void RndLightMgrCom::_AccumUntiledDeferredLight(
    RndContext& ctx,
    const RndSceneDrawParams& params,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    RndTextureBase* srcLightAccum) {
    ctx.SetRenderTargets(srcLightAccum, Frame(buffers).mDepthStencil);

    RndLightProbeParams probe = NoProbe();
    static const unsigned long sCombineSingleProbe =
        DataVarIndex(Symbol("combine_single_probe"), DataNode(1));
    if (DataVariable(sCombineSingleProbe).mValue.integer != 0
        && mCullResults.mProbes.size() == 1) {
        bool directional = true;
        for (RndLightEnvironCom* environ : mEnvironComs) {
            if (environ != nullptr
                && !environ->mRuntime.mHasCulledDirectionalLight) {
                directional = false;
                break;
            }
        }
        if (directional) {
            probe.mProbe = mCullResults.mProbes[0];
            probe.mStateIndices[0] = mCurProbeStateAIndex;
            probe.mStateIndices[1] = mCurProbeStateBIndex;
            probe.mStateBlend = mCurStateBlend;
        }
    }

    for (int type = 0; type < 3; ++type) {
        for (auto& list : mCullResults.mLights[type]) {
            for (RndLightCom* light : list) {
                if (type == kTiledLightsDirectional) {
                    light->_PreDrawNoComputeImpl(ctx, probe);
                } else {
                    light->_PreDrawNoComputeImpl(ctx, NoProbe());
                }
            }
        }
    }
    ctx._SetDepthClipEnabledImpl(false);
    SelectLightingShadingMode(ctx, params.mShadingMode);
    gRndDevice->mLighting.mDirectionalShader->SelectCommon(ctx, buffers);
    if (probe.mProbe == nullptr) {
        _AccumUntiledDeferredProbes(
            ctx, params, buffers, target, srcLightAccum);
    }
    for (RndLightEnvironCom* environ : mEnvironComs) {
        if (environ != nullptr) {
            environ->DrawDeferredLightNoCompute(ctx, buffers, target, probe);
        }
    }
    ctx._SetStencilModeImpl(0, 0, 0, 0);
    ctx.SetShadingMode(kShadingModeStandard);
    ctx._SetDepthClipEnabledImpl(true);
}

// Reconstructed from eboot.elf at 0x485E40.
void RndLightMgrCom::DrawShadowMaps(
    RndContext& ctx,
    RndSceneDrawer& drawer,
    const RndCameraContext& camera,
    const RndShowHideContext& showHide,
    VectorAdapter<PodVector<RndDrawInstance>>& instances,
    VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
    unsigned long index) {
    static Symbol sShadowsName;
    if (sShadowsName == Symbol()) {
        sShadowsName = Symbol("Shadows");
    }
    RndScopedGpuStatBlock shadows(ctx, sShadowsName.Str());
    static Symbol sGenerationName;
    if (sGenerationName == Symbol()) {
        sGenerationName = Symbol("Shadow Map Generation");
    }
    RndScopedGpuStatBlock generation(ctx, sGenerationName.Str());
    mCullResults.mShadowLights[index]->_DrawShadowMapImpl(
        ctx, drawer, camera, showHide, instances, sortable, *this);
}

// Reconstructed from eboot.elf at 0x485FC0.
long RndLightMgrCom::AcquireShadowContribution() {
    const std::uint64_t index = mNumShadowContributions;
    if (index >= static_cast<std::uint64_t>(gRndDevice->mSettings->mMaxShadowContribBuffers)) {
        return -1;
    }
    mNumShadowContributions = index + 1;
    return static_cast<long>(index);
}

// Reconstructed from eboot.elf at 0x486000. The warning's arguments are
// built although its output is compiled out.
long RndLightMgrCom::AcquireSpotShadowDepthLayers(unsigned long count) {
    const std::uint64_t first = mNumSpotShadowDepthLayers;
    const std::uint64_t end = first + count;
    if (end <= mQualitySettings[mSpotShadowConfigIndex].mMaxShadowCastingSpotlights) {
        mNumSpotShadowDepthLayers = end;
        return static_cast<long>(first);
    }
    static_cast<void>(mObject->MakeErrorName());
    static_cast<void>(RndQualityLevelName(static_cast<RndQualityLevel>(mSpotShadowConfigIndex)));
    return -1;
}

// Reconstructed from eboot.elf at 0x486060.
void RndLightMgrCom::GenerateAmbientOcclusion(
    RndContext& ctx,
    RndBufferCollection& buffers,
    RndCameraContext& camera,
    const RndSceneDrawParams& params) {
    mAmbientOcclusionGenerated = false;
    if ((gRndDevice->mCapabilities[kPlatformPS4].mFeatureFlags
         & kAsyncComputeFeature) == 0) {
        return;
    }
    RndSSAOCom* ssao = _GetAmbientOcclusion();
    if (ssao != nullptr
        && ssao->IsEnabled(gRndDevice->mSettings->mQualityLevel)) {
        ssao->GenerateAO(ctx, buffers, camera, params);
        mAmbientOcclusionGenerated = true;
    }
}

// Inlined into GenerateAmbientOcclusion at 0x48607E.
RndSSAOCom* RndLightMgrCom::_GetAmbientOcclusion() const {
    if (mAmbientOcclusion.mId == gNullObjectId.mId) {
        return nullptr;
    }
    const GameObject* object = mObject->mEntity->GetObject(mAmbientOcclusion);
    return object != nullptr ? object->GetCom<RndSSAOCom>() : nullptr;
}

// Reconstructed from eboot.elf at 0x486200. The tile counts round up.
void RndLightMgrCom::SetFwdLightingConstants(
    const Vector2i& size,
    RndShaderCBuffer& cbuffer) const {
    const RndConfig& config = *gRndDevice->mSettings;
    if (!config.mUseTiledLighting) {
        return;
    }
    const int tileSize = static_cast<int>(config.mLightTileSize);
    const int tilesX = size.x / tileSize + (size.x / tileSize * tileSize < size.x);
    const int tilesY = size.y / tileSize + (size.y / tileSize * tileSize < size.y);
    float* constants = reinterpret_cast<float*>(
        static_cast<char*>(cbuffer.mData)
        + 16 * gRndDevice->mShaderMgr.mTiledLightingParams);
    constants[0] = static_cast<float>(tilesX);
    constants[1] = static_cast<float>(tilesY);
    constants[2] = static_cast<float>(
        mLightBuffers[kTiledLightsDirectional].mNumPosLights);
    cbuffer.mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x4862B0.
void RndLightMgrCom::SetNoFwdLightingConstants(RndShaderCBuffer& cbuffer) {
    *reinterpret_cast<Vector3*>(
        static_cast<char*>(cbuffer.mData)
        + 16 * gRndDevice->mShaderMgr.mTiledLightingParams) = Vector3::sZero;
    cbuffer.mSyncPending = true;
}

// Inlined into TonemapScene at 0x48633D and UpdateDrawTarget at 0x4867C0.
// Operators 2 and 3 are Power and Linear/Power Hybrid.
bool RndLightMgrCom::_TonemapsScene() const {
    if (!mQualitySettings[mSpotShadowConfigIndex].mTonemappingEnabled
        || mTonemapping == 0) {
        return false;
    }
    return std::fabs(mTonemappingExposure - 1.0F) > 0.0001F
        || (mTonemappingOperator & ~1) == 2;
}

// Reconstructed from eboot.elf at 0x4862F0. The device settings can turn
// tonemapping off. The quad keeps the tonemap shader and the stencil test.
void RndLightMgrCom::TonemapScene(
    RndContext& ctx,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target) {
    if (!gRndDevice->mSettings->mTonemappingEnabled || !_TonemapsScene()) {
        return;
    }
    static Symbol sTonemapping;
    RndScopedGpuStatBlock stat(ctx, StatName(sTonemapping, "Tonemapping"));
    const RndConfig& config = *gRndDevice->mSettings;

    RndTonemapShaderBase::Params tonemap;
    tonemap.mSource = SrcLightAccum(buffers, target);
    tonemap.mOutput = DstLightAccum(buffers, target);
    tonemap.mTonemapOp = mTonemappingOperator;
    tonemap.mTonemapParams[0] = 1.0F / mTonemappingExposure;
    tonemap.mTonemapParams[1] = mTonemappingPower;

    const RndResourceState read = config.mUseTiledLighting
        ? kStateShaderResource
        : RndResourceState::kPixelShaderResource;
    FixedVector<RndResourceBarrier, 2> barriers;
    barriers.push_back(Transition(
        tonemap.mOutput, read, RndResourceState::kRenderTarget));
    barriers.push_back(Transition(
        tonemap.mSource, RndResourceState::kRenderTarget, read));
    ctx._ResourceBarrierImpl(barriers.size(), barriers.mData);

    RndContext::RenderTargetParams targets;
    RndContext::RenderTargetParams::Target output;
    output.mTexture = tonemap.mOutput;
    output.mClearMode = 2;
    targets.mTargets.push_back(output);
    targets.mDepthTexture = Frame(buffers).mDepthStencil;
    ctx.SetRenderTargets(targets);
    ctx._SetDepthModeImpl(0);
    gRndDevice->mLighting.mTonemapShader->Select(ctx, tonemap);
    ctx._SetStencilModeImpl(
        kSceneStencilMode, 0, kSceneStencilReadMask, 0);

    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    RndDrawUtl::DrawQuad2D(ctx, quad);
    ctx._SetStencilModeImpl(0, 0, 0, 0);
    SwapLightAccum(target);
}

// Reconstructed from eboot.elf at 0x4867C0.
void RndLightMgrCom::UpdateDrawTarget(RndSceneDrawTarget& target) const {
    if (_TonemapsScene()) {
        SwapLightAccum(target);
    }
}

// Reconstructed from eboot.elf at 0x489F70.
void RndLightMgrCom::_PostCreate() {
    const Symbol defaultState = RndLightProbeCom::GetDefaultStateName();
    mProbeStates._Insert(0, &defaultState);
    _SyncCurProbeStateIndices();
    mProbeTexArraysDirty = true;
    for (RndLightProbeCom* probe : mProbes) {
        probe->StateInserted(0);
    }
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x48A090.
void RndLightMgrCom::_InsertProbeState(unsigned long index, Symbol name) {
    mProbeStates._Insert(index, &name);
    _SyncCurProbeStateIndices();
    mProbeTexArraysDirty = true;
    for (RndLightProbeCom* probe : mProbes) {
        probe->StateInserted(index);
    }
}

// Reconstructed from eboot.elf at 0x48A1A0.
void RndLightMgrCom::_PreDestroy(DestroyType type) {
    static_cast<void>(type);
}

// Reconstructed from eboot.elf at 0x48A1B0. The probes of an old scene
// each held their state names; they are merged into "states".
bool RndLightMgrCom::_OnResourcesLoaded() {
    mSpotShadowConfigIndex = static_cast<std::int32_t>(gRndDevice->mSettings->mQualityLevel);
    _InitBuffers();
    if (mUpgradeProbeStates) {
        mUpgradeProbeStates = false;
        mProbeStates.Resize(0);
        const Symbol defaultState = RndLightProbeCom::GetDefaultStateName();
        mProbeStates._Insert(mProbeStates.size(), &defaultState);
        const Entity* entity = mObject->mEntity;
        for (GameObject* object = entity->BeginObject(); object != nullptr;
             object = entity->NextObject(object, Symbol())) {
            RndLightProbeCom* probe = object->GetCom<RndLightProbeCom>();
            if (probe == nullptr) {
                continue;
            }
            for (unsigned int i = 0; i < probe->mStateNames.size(); ++i) {
                const Symbol name = probe->mStateNames[i];
                if (Find(mProbeStates, name) == static_cast<unsigned long>(-1)) {
                    mProbeStates._Insert(mProbeStates.size(), &name);
                }
            }
            probe->mStateNames.Resize(0);
        }
    }
    _SyncCurProbeStateIndices();
    return true;
}

// Reconstructed from eboot.elf at 0x48A400.
void RndLightMgrCom::_InitBuffers() {
    const auto& settings = *TheRndDevice()->mSettings;

    if (settings.mUseTiledLighting) {
        mLightBuffers[kTiledLightsPoint].mBuffer = NewTiledLightBuffer(
            208,
            static_cast<unsigned long>(settings.mMaxPointLights),
            "Point Lights");
        mLightBuffers[kTiledLightsSpot].mBuffer = NewTiledLightBuffer(
            352,
            static_cast<unsigned long>(settings.mMaxSpotLights),
            "Spotlights");
        mLightBuffers[kTiledLightsDirectional].mBuffer = NewTiledLightBuffer(
            112,
            static_cast<unsigned long>(settings.mMaxDirectionalLights),
            "Directional Lights");
        mLightProbeBuffer = NewTiledLightBuffer(
            96,
            static_cast<unsigned long>(settings.mMaxLightProbes),
            "Light Probes");

        const auto sliceZeroCapacity = settings.mMaxPointLights +
            settings.mMaxSpotLights + settings.mMaxLightProbes;
        mSliceZeroLightIdBuffer = NewTiledLightBuffer(
            sizeof(std::uint32_t),
            static_cast<unsigned long>(sliceZeroCapacity),
            "Slice Zero Ligth Ids");
    }

    _SyncSpotShadowDepthTexArray();
}

// Reconstructed from eboot.elf at 0x48A5A0.
void RndLightMgrCom::_SyncCurProbeStateIndices() {
    mCurProbeStateAIndex = Find(mProbeStates, mCurStateA);
    mCurProbeStateBIndex = Find(mProbeStates, mCurStateB);
}

// Reconstructed from eboot.elf at 0x48A640.
void RndLightMgrCom::_Exit(DestroyType type) {
    if (type > kDestroyEditorInstance) {
        for (RndLightEnvironCom* environ : mEnvironComs) {
            if (environ != nullptr) {
                environ->mRuntime.mRegistered = false;
            }
        }
    }
    mEnvironments.Resize(0);
    mEnvironComs.clear();
    mLights.clear();
    mProbes.clear();
    mProbeIds.Resize(0);
}

// Reconstructed from eboot.elf at 0x48A6E0. The default environment is
// kept first in "environments", after a null slot when it is unset.
void RndLightMgrCom::_Poll() {
    if (mDefaultEnvironDirty) {
        mDefaultEnvironDirty = false;
        unsigned long count = 0;
        if (mEnvironments.size() != 0) {
            GameObjectId first = mEnvironments[0];
            if (first.mId != mDefaultEnvironment.mId) {
                if (first.mId != gNullObjectId.mId) {
                    mEnvironments._Insert(0, &gNullObjectId);
                    first = gNullObjectId;
                }
                if (mDefaultEnvironment.mId != first.mId) {
                    const unsigned long index = Find(mEnvironments, mDefaultEnvironment);
                    if (index != static_cast<unsigned long>(-1)) {
                        mEnvironments[0] = mDefaultEnvironment;
                        RemoveAt(mEnvironments, index);
                    }
                }
            }
            count = mEnvironments.size();
        }
        _ReindexEnvirons(0, count);
    }

    for (const RenamedProbeState& renamed : mRenamedProbeStates) {
        const Symbol newName = mProbeStates[renamed.mIndex];
        if (renamed.mOldName != Symbol() && Find(mProbeStates, renamed.mOldName) == static_cast<unsigned long>(-1)) {
            if (mCurStateA == renamed.mOldName) {
                mCurStateA = newName;
            }
            if (mCurStateB == renamed.mOldName) {
                mCurStateB = newName;
            }
            if (mCaptureState == renamed.mOldName) {
                mCaptureState = newName;
            }
        }
        for (unsigned long i = 0; i < mProbes.size(); ++i) {
            mProbes[i]->StateRenamed(renamed.mIndex, renamed.mOldName, newName);
        }
    }
    mRenamedProbeStates.clear();

    _SyncCookieTexArrays();
    if (mProbeTexArraysDirty) {
        _RebuildProbeTexArrays();
    }

    const std::int32_t quality = static_cast<std::int32_t>(gRndDevice->mSettings->mQualityLevel);
    if (quality != mSpotShadowConfigIndex) {
        const QualitySettings& previous = mQualitySettings[mSpotShadowConfigIndex];
        mSpotShadowConfigIndex = quality;
        const QualitySettings& current = mQualitySettings[quality];
        if (previous.mMaxShadowCastingSpotlights != current.mMaxShadowCastingSpotlights ||
            (previous.mMaxShadowCastingSpotlights != 0 &&
             previous.mSpotlightShadowmapResolution != current.mSpotlightShadowmapResolution)) {
            mSpotShadowDepthTexArrayDirty = true;
        }
    }
    if (mSpotShadowDepthTexArrayDirty) {
        _SyncSpotShadowDepthTexArray();
    }
    mLightsCulled = false;
}

// Reconstructed from eboot.elf at 0x48AB30.
void RndLightMgrCom::_SyncSpotShadowDepthTexArray() {
    mSpotShadowDepthTexArrayDirty = false;
    SafeDelete(mSpotShadowDepthTexArray);

    const QualitySettings& config = mQualitySettings[mSpotShadowConfigIndex];
    if (config.mMaxShadowCastingSpotlights == 0) {
        return;
    }

    const RndDataFormatInfo depthFormat{16, 12, 0, 1, -1};
    const auto dataFormat = RndFindSupportedDataFormat(depthFormat, kPlatformPS4);
    const auto resolution = config.mSpotlightShadowmapResolution < kSpotShadowResolutions.size()
        ? kSpotShadowResolutions[config.mSpotlightShadowmapResolution]
        : std::numeric_limits<std::uint32_t>::max();

    RndTextureArray2D::Description desc;
    desc.mRequestedFormat.mUsage = 2;
    desc.mRequestedFormat.mSettings[5] = 1;
    desc.mRequestedFormat.mWrapMode = 1;
    desc.mRequestedFormat.mFilterMode = 1;
    desc.mRequestedFormat.mFlags = 2;
    desc.mName = "Spot Shadow Depth TexArray";

    const auto numLayers = static_cast<unsigned long>(config.mMaxShadowCastingSpotlights);
    desc.mPixels.resize(numLayers);
    const auto size = static_cast<int>(resolution);
    for (unsigned long index = 0; index < numLayers; ++index) {
        desc.mPixels[index].CreateEmpty(size, size, 1, dataFormat);
    }

    mSpotShadowDepthTexArray = RndTextureArray2D::New(desc);
}

// Reconstructed from eboot.elf at 0x48AD90.
void RndLightMgrCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x48ADA0.
void RndLightMgrCom::_EditExit(DestroyType type) {
    if (type > kDestroyEditorInstance) {
        for (RndLightEnvironCom* environ : mEnvironComs) {
            if (environ != nullptr) {
                environ->mRuntime.mRegistered = false;
            }
        }
    }
    mEnvironments.Resize(0);
    mEnvironComs.clear();
    mLights.clear();
    mProbes.clear();
    mProbeIds.Resize(0);
}

// Reconstructed from eboot.elf at 0x48AE40.
void RndLightMgrCom::_SyncMasterIntensityMult() {
    for (RndLightCom* light : mLights) {
        light->mRuntime.mMasterIntensityMult = mMasterIntensityMult;
    }
}

// Reconstructed from eboot.elf at 0x48B080.
void RndLightMgrCom::CullResults::ReserveFor(const CullResults& other) {
    for (int type = 0; type < 3; ++type) {
        mLights[type][0].reserve(other.mLights[type][0].size());
        mLights[type][1].reserve(other.mLights[type][1].size());
    }
    mProbes.reserve(other.mProbes.size());
    mShadowLights.reserve(other.mShadowLights.size());
}

// Reconstructed from eboot.elf at 0x48B870. Only square pixels at least
// "cookie_texture_size" wide qualify; a smaller cookie takes the error
// cookie's pixels.
const RndPixelData* RndLightMgrCom::_GetCookieArrayPixels(
    const char* name,
    int dataFormat,
    const RndPixelData& pixels,
    RndPixelData& scratch) const {
    static_cast<void>(name);
    const int size = static_cast<int>(mCookieTextureSize);
    const RndPixelData* level = FindCookieMip(pixels, size);
    if (level != nullptr) {
        return level;
    }
    RndTexture2D* error = gRndDevice->mLighting.GetErrorLightCookie();
    if (error != nullptr) {
        level = FindCookieMip(error->mPixels, size);
        if (level != nullptr) {
            return level;
        }
    }
    scratch.Create(Vector3i{size, size, 1}, dataFormat, nullptr);
    scratch.CreateMips();
    for (RndPixelData* mip = &scratch; mip != nullptr; mip = mip->mMip) {
        std::memset(mip->mBuffer, 0, mip->mBufferSize);
    }
    return &scratch;
}

// Reconstructed from eboot.elf at 0x48B9D0. One scratch level serves every
// face.
RndPixelDataCube& RndLightMgrCom::_GetCookieArrayPixels(
    const char* name,
    int dataFormat,
    const RndPixelDataCube& pixels,
    RndPixelDataCube& cube) const {
    RndPixelData scratch;
    for (int face = 0; face < RndPixelDataCube::kNumFaces; ++face) {
        // The binary's RndPixelData::operator= (0x6832B0).
        cube.mFaces[face].CopyFrom(
            *_GetCookieArrayPixels(
                name, dataFormat, pixels.mFaces[face], scratch),
            true);
    }
    return cube;
}

// Reconstructed from eboot.elf at 0x48B300. The probes add into the
// target where the stencil marks lit pixels.
void RndLightMgrCom::_AccumUntiledDeferredProbes(
    RndContext& ctx,
    const RndSceneDrawParams& params,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    RndTextureBase* srcLightAccum) {
    static_cast<void>(params);
    static_cast<void>(target);
    static Symbol sAccumulateProbes;
    RndScopedGpuStatBlock stat(
        ctx, StatName(sAccumulateProbes, "Accumulate Probes"));
    if (mCullResults.mProbes.empty()) {
        return;
    }
    ctx._SetStencilModeImpl(kProbeStencilMode, 2, 1, 0);
    ctx.mBlendMode = RndBlendMode::kAdd;
    ctx._SetBlendModeImpl(RndBlendMode::kAdd, Hmx::Color::GetWhite());
    RndBufferCollection::FrameIntervalBuffers& frame = Frame(buffers);
    RndTextureBase* ao = mAmbientOcclusionGenerated ? frame.mAO : nullptr;
    if (mCullResults.mProbes.size() == 1 && mMasterIntensityMult == 1.0F) {
        mCullResults.mProbes[0]->DrawDeferredLightNoCompute(
            ctx, mCurProbeStateAIndex, mCurProbeStateBIndex, mCurStateBlend,
            ao);
        return;
    }

    FixedVector<RndResourceBarrier, 1> barriers;
    barriers.push_back(Transition(
        buffers.mLightProbeAccum,
        RndResourceState::kPixelShaderResource,
        RndResourceState::kRenderTarget));
    ctx._ResourceBarrierImpl(barriers.size(), barriers.mData);

    RndContext::RenderTargetParams targets;
    RndContext::RenderTargetParams::Target accum;
    accum.mTexture = buffers.mLightProbeAccum;
    accum.mClearMode = 1;
    targets.mTargets.push_back(accum);
    targets.mDepthTexture = frame.mDepthStencil;
    ctx.SetRenderTargets(targets);
    for (RndLightProbeCom* probe : mCullResults.mProbes) {
        probe->DrawDeferredLightNoCompute(
            ctx, mCurProbeStateAIndex, mCurProbeStateBIndex, mCurStateBlend,
            ao);
    }
    barriers[0].mBefore = RndResourceState::kRenderTarget;
    barriers[0].mAfter = RndResourceState::kPixelShaderResource;
    ctx._ResourceBarrierImpl(barriers.size(), barriers.mData);

    ctx.SetRenderTargets(srcLightAccum, frame.mDepthStencil);
    RndLightProbeDeferredAccumShader::Params accumParams;
    accumParams.mIllumType = 0;
    accumParams.mProbeAccumBuffer = buffers.mLightProbeAccum;
    accumParams.mProbeIntensityMult = mMasterIntensityMult;
    gRndDevice->mLighting.mProbeAccumShader->Select(ctx, accumParams);
    ctx._SetCullModeImpl(kCullNone);
    ctx._SetDepthModeImpl(0);
    RndDrawUtl::Quad2DParams quad;
    quad.mKeepShader = true;
    quad.mKeepState = true;
    RndDrawUtl::DrawQuad2D(ctx, quad);
}

// Reconstructed from eboot.elf at 0x48BB20. The current, blended and
// capture states fall back to no state when theirs was removed.
void RndLightMgrCom::_RemoveProbeState(unsigned long index) {
    RemoveAt(mProbeStates, index);
    _SyncCurProbeStateIndices();
    if (mCurProbeStateAIndex == static_cast<std::uint64_t>(-1)) {
        mCurStateA = Symbol("");
    }
    if (mCurProbeStateBIndex == static_cast<std::uint64_t>(-1)) {
        mCurStateB = Symbol("");
    }
    if (Find(mProbeStates, mCaptureState) == static_cast<unsigned long>(-1)) {
        mCaptureState = Symbol("");
    }
    mProbeTexArraysDirty = true;
    for (RndLightProbeCom* probe : mProbes) {
        probe->StateRemoved(index);
    }
}

// Reconstructed from eboot.elf at 0x48BD10. No state comes first.
void RndLightMgrCom::GetAllowedProbeStates(
    eastl::vector<AllowedValue<Symbol>>& values) const {
    const unsigned long count = mProbeStates.size() + 1;
    if (values.capacity() < count) {
        values.reserve(count);
    }
    values.push_back(AllowedValue<Symbol>{
        Symbol(""),
        String(RndLightProbeCom::GetNoneStateDisplayName().Str()),
        String("")});
    for (const Symbol& state : mProbeStates) {
        values.push_back(
            AllowedValue<Symbol>{state, String(state.Str()), String("")});
    }
}

// Reconstructed from eboot.elf at 0x48BEE0. The lights capture at full
// intensity and every probe shows the capture state alone; each bounce
// captures all probes before committing them, so later bounces see the
// earlier ones.
void RndLightMgrCom::CaptureAllProbes() {
    const unsigned long numProbes = mProbes.size();
    if (numProbes == 0 || mProbeStates.size() == 0) {
        return;
    }
    const unsigned long index = Find(mProbeStates, mCaptureState);
    if (index == static_cast<unsigned long>(-1)) {
        return;
    }
    static const char* const kTask = "LightProbeCaptureAll";
    if (HasLoadProgressListeners()) {
        const unsigned int total =
            static_cast<unsigned int>(numProbes * mCaptureNumBounces);
        FormatString text("Capturing %d Light Probes (State '%s')");
        text << numProbes << mCaptureState;
        LoadProgressBegin(kTask, text.Str(), total, 0);
    }

    const float masterIntensityMult = mMasterIntensityMult;
    mMasterIntensityMult = 1.0F;
    for (RndLightCom* light : mLights) {
        light->mRuntime.mMasterIntensityMult = 1.0F;
    }
    const Symbol curStateA = mCurStateA;
    const Symbol curStateB = mCurStateB;
    const float curStateBlend = mCurStateBlend;
    mCurStateA = mCaptureState;
    mCurStateB = mCaptureState;
    mCurStateBlend = 0.0F;
    _SyncCurProbeStateIndices();

    for (RndLightProbeCom* probe : mProbes) {
        probe->ZeroCaptureResults(mCaptureState, index);
    }
    _RebuildProbeTexArrays();
    unsigned int step = 0;
    for (std::uint64_t bounce = 0; bounce < mCaptureNumBounces; ++bounce) {
        for (RndLightProbeCom* probe : mProbes) {
            probe->Capture(mCaptureState, index);
            if (HasLoadProgressListeners()) {
                LoadProgressSet(kTask, step++);
            }
        }
        for (RndLightProbeCom* probe : mProbes) {
            probe->CommitCaptureResults(mCaptureState, index);
        }
        _RebuildProbeTexArrays();
    }
    if (HasLoadProgressListeners()) {
        LoadProgressEnd(kTask);
    }

    mMasterIntensityMult = masterIntensityMult;
    for (RndLightCom* light : mLights) {
        light->mRuntime.mMasterIntensityMult = masterIntensityMult;
    }
    mCurStateA = curStateA;
    mCurStateB = curStateB;
    mCurStateBlend = curStateBlend;
    _SyncCurProbeStateIndices();
}

// Reconstructed from eboot.elf at 0x48C3C0.
void RndLightMgrCom::ZeroAllProbes() {
    const unsigned long index = Find(mProbeStates, mCaptureState);
    if (index == static_cast<unsigned long>(-1)) {
        return;
    }
    for (unsigned int i = 0; i < mProbeIds.size(); ++i) {
        mProbes[i]->ZeroCaptureResults(mCaptureState, index);
    }
    _RebuildProbeTexArrays();
}

// Inlined into CaptureAllProbes and ZeroAllProbes; _Poll rebuilds the
// arrays the same way.
void RndLightMgrCom::_RebuildProbeTexArrays() {
    mProbeTexArraysDirty = false;
    if (gRndDevice->mSettings->mUseTiledLighting) {
        SafeDelete(mProbeTexArrays[0]);
        SafeDelete(mProbeTexArrays[1]);
        RndLightProbeCom::SyncTexarrays(
            mObject->mEntity, mProbes, mProbeIds, mProbeStates,
            mProbeTexArrays);
    }
}

// Reconstructed from eboot.elf at 0x48C8B0.
DataNode RndLightMgrCom::_OnLeavingPlayMode() {
    mCookieTexArraysDirty = true;
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x48C8D0.
DataNode RndLightMgrCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol sLeavingPlayMode;
    if (sLeavingPlayMode == Symbol()) {
        sLeavingPlayMode = Symbol("leaving_playmode");
    }
    if (type != sLeavingPlayMode) {
        static Symbol sLeavingRecordMode;
        if (sLeavingRecordMode == Symbol()) {
            sLeavingRecordMode = Symbol("leaving_record_mode");
        }
        if (type != sLeavingRecordMode) {
            return DataNode();
        }
    }
    return _OnLeavingPlayMode();
}

// Reconstructed from eboot.elf at 0x48CA50.
Symbol RndLightMgrCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x48CA60.
Symbol RndLightMgrCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x48CA70.
int RndLightMgrCom::CurrentRev() const {
    return const_cast<RndLightMgrCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x48CA90.
bool RndLightMgrCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x48CAC0.
Component* RndLightMgrCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x48CAD0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndLightMgrCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightMgrCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightMgrCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x48CCD0.
PropRegistry& RndLightMgrCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x48CCE0.
ComMetaData& RndLightMgrCom::_GetMetaData() {
    return sMetaData;
}
