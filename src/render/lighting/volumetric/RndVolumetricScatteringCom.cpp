// render/RndVolumetricScatteringCom.o (0x453C00 to 0x45760F). The object
// also emits the templates of PropArray<QualitySettings> (0x453DE0,
// 0x4568E0-0x456B80), the class description's type registration
// (0x456000), the quality names (0x4561B0, 0x4561D0) and the registry's
// property callbacks (0x456740-0x457500), among them the revision upgrade
// (0x456C10) that moves the old "fog_start_dist" and "fog_end_dist" into
// the atmosphere's distances.
#include "render/lighting/volumetric/RndVolumetricScatteringCom.h"

#include <new>

#include "math/random/Rand.h"
#include "math/vector/Vector2.h"
#include "math/vector/Vector2i.h"
#include "math/vector/Vector3.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/context/RndResourceBarrier.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/volumetric/RndCShaderVScatAccumScattering.h"
#include "render/lighting/volumetric/RndCShaderVScatCalcDensityInscattering.h"
#include "render/lighting/volumetric/RndCShaderVScatDeferred.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTexture1D.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTexture3D.h"
#include "utl/containers/FixedVector.h"

// The object's statics, in the order of its static initializer (0x457510).
// The three ints it first sets come from a shared header and are not
// modelled.
Symbol RndVolumetricScatteringCom::sId("VolumetricScattering");
Symbol RndVolumetricScatteringCom::sClassName("VolumetricScattering");
PropRegistry RndVolumetricScatteringCom::sPropRegistry;
ComMetaData RndVolumetricScatteringCom::sMetaData;

namespace {

// Header statics the initializer constructs last: the size the height
// density is baked at (read by _CreateTexture, 0x456460), and the empty
// waveform resource a new height density starts from. Names not in the
// reference map.
[[maybe_unused]] const Vector2i gWaveformTextureSize = {128, 8};  // 0x1A76018
ResourcePtr<Resource> gEmptyWaveformResource;  // 0x1A76020

// The fog range is kept at least this long.
constexpr float kMinFogRange = 0.0001F;
// The scale of a fog range of zero.
constexpr float kNoRangeScale = 10000.0F;

// The fog's start and end distances for the camera: the start clamped to
// its near and far planes, and the end to the far plane, or the far plane
// itself. Inlined into each pass. Name not in the reference map.
struct FogRange {
    float mStart;
    float mEnd;
};

FogRange CalcFogRange(
    const RndVolumetricScatteringCom& com,
    const RndCameraContext& camera) {
    const RndCameraSettings& settings = camera.GetCameraSettings();
    const float farPlane = settings.mFarPlane;
    FogRange range;
    range.mStart = farPlane < com.mStartDist
        ? farPlane
        : (com.mStartDist > settings.mNearPlane ? com.mStartDist
                                                : settings.mNearPlane);
    range.mEnd = farPlane;
    if (!com.mUseCameraEndDist) {
        range.mEnd = farPlane < com.mEndDist
            ? farPlane
            : (range.mStart > com.mEndDist ? range.mStart : com.mEndDist);
    }
    const float minEnd = range.mStart + kMinFogRange;
    range.mEnd = minEnd > range.mEnd ? minEnd : range.mEnd;
    return range;
}

// The resolution of the device's quality level.
int CurrentResolution(const RndVolumetricScatteringCom& com) {
    const auto level = TheRndDevice()->mSettings->mQualityLevel;
    return com.mQualitySettings[static_cast<int>(level)].mResolution;
}

// The light accumulation buffer the target selects, or the frame
// interval's own one outside a scene context. Inlined at each use.
RndTextureBase* LightAccum(
    const RndBufferCollection& buffers,
    unsigned long index) {
    if (buffers.mActiveSceneContext != 0) {
        return buffers.mLightAccum[index];
    }
    return buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval]
        .mPartialLightAccum;
}

const RndBufferCollection::FrameIntervalBuffers& Frame(
    const RndBufferCollection& buffers) {
    return buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
}

// A one-barrier list over the whole resource.
void SetBarrier(
    RndResourceBarrier& barrier,
    RndTextureBase* resource,
    RndResourceState before,
    RndResourceState after) {
    barrier.mResource = resource;
    barrier.mSubresource = ~0UL;
    barrier.mBefore = before;
    barrier.mAfter = after;
}

// Unbinds the compute stage's read-write and source textures after a
// dispatch, resetting the stage's slot limits.
void DeselectComputeTextures(RndContext& context) {
    constexpr unsigned int kComputeStage = 1U << kShaderProgramCompute;
    context._DeselectAllReadWriteTexturesImpl(kComputeStage);
    context.mInputSlotLimits[kShaderProgramCompute] = 0;
    context._DeselectAllSourceTexturesImpl(kComputeStage);
    context.mOutputSlotLimits[kShaderProgramCompute] = 0;
}

// The waveform's random phases, drawn when it is constructed.
void DrawPhases(RndVolumetricScatteringCom::HeightDensity& waveform) {
    const float first = gRand.Float();
    waveform.mEvalData[0] = first;
    waveform.mEvalData[1] = gRand.Float() - first;
    waveform.mRand = &gRand;
}

}  // namespace

// Reconstructed from eboot.elf at 0x453C00.
RndVolumetricScatteringCom::RndVolumetricScatteringCom()
    : mFogDensity(0.2F),
      mUseCameraEndDist(false),
      mHeightRangeBegin(0.0F),
      mHeightRangeEnd(5.0F),
      mHeightDensity{
          ResourcePtr<Resource>(gEmptyWaveformResource.Get()),
          {0.0F, 0.0F},
          nullptr,
          ResourcePtr<Resource>(),
          {0, 0}},
      mVolumetricLightIntensity(1.0F),
      mRuntimeData{0.0F, false, ResourcePtr<RndTexture1DResource>()} {
    DrawPhases(mHeightDensity);
}

// Reconstructed from eboot.elf at 0x453D40. The deleting destructor is at
// 0x453E40.
RndVolumetricScatteringCom::~RndVolumetricScatteringCom() {}

// Reconstructed from eboot.elf at 0x453E90.
bool RndVolumetricScatteringCom::IsEnabled(RndQualityLevel level) const {
    return mQualitySettings[static_cast<int>(level)].mEnabled;
}

// Reconstructed from eboot.elf at 0x453EB0. The volume is written by the
// density pass and read by the accumulation, whose output the deferred
// pass reads.
void RndVolumetricScatteringCom::BeginAsyncUpdate(
    RndContext& context,
    const RndCameraContext& camera,
    const RndCameraContext* stereoCamera,
    RndBufferCollection& buffers,
    RndBufferCollection* stereoBuffers,
    RndLightMgrCom* lightMgr,
    RndTextureBase* skyTexture,
    bool useSceneMask) const {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Volumetric Scattering");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndDevice* const device = TheRndDevice();
    if (skyTexture == nullptr) {
        skyTexture = device->mDefaults.mTextures2D[kDefaultTextureBlack];
    }
    const FogRange range = CalcFogRange(*this, camera);
    const int resolution = CurrentResolution(*this);
    RndTexture3D* const inscattering = stereoBuffers != nullptr
        ? Frame(*stereoBuffers).mStereoVScatInscattering[resolution]
        : Frame(buffers).mVScatInscattering[resolution];
    const float densityStep = (range.mEnd - range.mStart)
        / static_cast<float>(inscattering->mBaseDesc.mDepth)
        * mRuntimeData.mFogDensity;
    RndShaderMgr& shaders = device->mShaderMgr;
    FixedVector<RndResourceBarrier, 1> barriers;
    barriers.resize(1);

    if (stereoBuffers == nullptr
        || camera.mTargetMode == kTargetModeLeftEye) {
        static Symbol sDensityStatName;
        if (sDensityStatName == Symbol()) {
            sDensityStatName = Symbol("Compute Density Inscattering");
        }
        RndScopedGpuStatBlock densityStatBlock(
            context, sDensityStatName.Str());
        SetBarrier(
            barriers[0],
            inscattering,
            RndResourceState::kNonPixelShaderResource,
            RndResourceState::kUnorderedAccess);
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);

        RndCShaderVScatCalcDensityInscattering::Params density;
        density.mCamera = stereoBuffers != nullptr ? stereoCamera : &camera;
        density.mDensityStep = densityStep;
        density.mLightIntensity = mVolumetricLightIntensity;
        density.mStartDist = range.mStart;
        density.mEndDist = range.mEnd;
        density.mLightBuffers = lightMgr->mLightBuffers;
        density.mSpotShadowMaps = lightMgr->mSpotShadowDepthTexArray;
        density.mBuffers = &buffers;
        density.mOutput = inscattering;
        density.mSkyTexture = skyTexture;
        density.mHeightRangeBegin = mHeightRangeBegin;
        density.mHeightRangeEnd = mHeightRangeEnd;
        RndTexture1DResource* const waveform =
            mRuntimeData.mHeightDensityTexture.Get();
        density.mHeightWaveform = waveform != nullptr
            ? static_cast<RndTextureBase*>(waveform->mTexture)
            : device->mDefaults.mTextures1D[kDefaultTextureBlack];
        density.mIsStereo = stereoBuffers != nullptr;
        shaders.mVScatCalcDensityInscatteringCShader->Dispatch(
            context, density);

        barriers[0].mBefore = RndResourceState::kUnorderedAccess;
        barriers[0].mAfter = RndResourceState::kNonPixelShaderResource;
        context._ResourceBarrierImpl(barriers.size(), barriers.mData);
    }

    static Symbol sAccumStatName;
    if (sAccumStatName == Symbol()) {
        sAccumStatName = Symbol("Accumulate Scattering");
    }
    RndScopedGpuStatBlock accumStatBlock(context, sAccumStatName.Str());
    const RndBufferCollection::FrameIntervalBuffers& frame = Frame(buffers);
    RndTexture3D* const accum = frame.mVScatAccumScattering[resolution];
    SetBarrier(
        barriers[0],
        accum,
        RndResourceState::kAllShaderResource,
        RndResourceState::kUnorderedAccess);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
    DeselectComputeTextures(context);

    RndCShaderVScatAccumScattering::Params scattering;
    scattering.mScreenSize = buffers.mSize;
    scattering.mDensityInscattering = inscattering;
    scattering.mTiledDepthRange = frame.mTiledDepthRange;
    scattering.mSceneMask =
        useSceneMask ? buffers.mTiledSceneMask[1] : nullptr;
    scattering.mOutput = accum;
    scattering.mStartDist = range.mStart;
    scattering.mEndDist = range.mEnd;
    scattering.mStereoEye = stereoBuffers != nullptr
        ? camera.mTargetMode - kTargetModeLeftEye
        : -1;
    scattering.mStereoCamera = stereoCamera;
    scattering.mCamera = &camera;
    shaders.mVScatAccumScatteringCShader->Dispatch(context, scattering);

    barriers[0].mBefore = RndResourceState::kUnorderedAccess;
    barriers[0].mAfter = RndResourceState::kAllShaderResource;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
}

// Reconstructed from eboot.elf at 0x454560.
void RndVolumetricScatteringCom::EndAsyncUpdate(
    RndContext& context,
    const RndCameraContext& camera,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target) const {
    static_cast<void>(context);
    static_cast<void>(camera);
    static_cast<void>(buffers);
    target.mScatteringResolution = CurrentResolution(*this);
}

// Reconstructed from eboot.elf at 0x454590. Every pass's source textures
// are unbound first. The destination is read and written by the compute
// pass; the source returns from a render target to a shader resource.
void RndVolumetricScatteringCom::ApplyDeferred(
    RndContext& context,
    const RndCameraContext& camera,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    bool useSceneMask) {
    if (context.mTargetMode == kTargetModeCube) {
        return;
    }
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Volumetric Scattering");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    const FogRange range = CalcFogRange(*this, camera);
    constexpr unsigned int kAllStages = 0x3F;
    context._DeselectAllSourceTexturesImpl(kAllStages);
    for (unsigned long& limit : context.mOutputSlotLimits) {
        limit = 0;
    }
    const RndBufferCollection::FrameIntervalBuffers& frame = Frame(buffers);
    RndTexture3D* const accum =
        frame.mVScatAccumScattering[CurrentResolution(*this)];
    RndTextureBase* const source = LightAccum(buffers, target.mSrcLightAccum);
    RndTextureBase* const dest = LightAccum(buffers, target.mDstLightAccum);
    FixedVector<RndResourceBarrier, 2> barriers;
    barriers.resize(2);
    SetBarrier(
        barriers[0],
        dest,
        RndResourceState::kAllShaderResource,
        RndResourceState::kUnorderedAccess);
    SetBarrier(
        barriers[1],
        source,
        RndResourceState::kRenderTarget,
        RndResourceState::kAllShaderResource);
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);

    TheRndDevice()->mShaderMgr.mVScatDeferredCShader->Dispatch(
        context,
        accum,
        source,
        dest,
        frame.mLinearDepth,
        range.mStart,
        range.mEnd,
        mRuntimeData.mFogDensity,
        useSceneMask ? buffers.mTiledSceneMask[1] : nullptr);

    barriers.resize(1);
    barriers[0].mBefore = RndResourceState::kUnorderedAccess;
    barriers[0].mAfter = RndResourceState::kRenderTarget;
    context._ResourceBarrierImpl(barriers.size(), barriers.mData);
    const unsigned long swapped = target.mSrcLightAccum;
    target.mSrcLightAccum = target.mDstLightAccum;
    target.mDstLightAccum = swapped;
    target.mResult = nullptr;
}

// Reconstructed from eboot.elf at 0x454900.
void RndVolumetricScatteringCom::SkipApplyDeferred(
    RndSceneDrawTarget& target) const {
    target.mScatteringResolution = CurrentResolution(*this);
    const unsigned long swapped = target.mSrcLightAccum;
    target.mSrcLightAccum = target.mDstLightAccum;
    target.mDstLightAccum = swapped;
    target.mResult = nullptr;
}

// Reconstructed from eboot.elf at 0x454940. The first constant is one, the
// fog's reciprocal range and its start scaled by it and negated; the
// second the end distance and the scaled density.
void RndVolumetricScatteringCom::SetFwdShadingConstants(
    const RndCameraContext& camera,
    RndShaderCBuffer& cbuffer) const {
    const FogRange range = CalcFogRange(*this, camera);
    const float length = range.mEnd - range.mStart;
    const float scale = length != 0.0F ? 1.0F / length : kNoRangeScale;
    const RndShaderMgr& shaders = TheRndDevice()->mShaderMgr;
    auto* params0 = static_cast<float*>(RndShaderDrawUtl::GetCBufferMember(
        cbuffer, shaders.mVolumetricParams0));
    params0[0] = 1.0F;
    params0[1] = scale;
    params0[2] = -(range.mStart * scale);
    auto* params1 = static_cast<float*>(RndShaderDrawUtl::GetCBufferMember(
        cbuffer, shaders.mVolumetricParams1));
    params1[0] = range.mEnd;
    params1[1] = mRuntimeData.mFogDensity;
    params1[2] = 0.0F;
    cbuffer.mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x454A10.
void RndVolumetricScatteringCom::SetNoAtmosphereFwdShadingConstants(
    RndShaderCBuffer& cbuffer) {
    const RndShaderMgr& shaders = TheRndDevice()->mShaderMgr;
    *static_cast<Vector3*>(RndShaderDrawUtl::GetCBufferMember(
        cbuffer, shaders.mVolumetricParams0)) = Vector3::sZero;
    *static_cast<Vector2*>(RndShaderDrawUtl::GetCBufferMember(
        cbuffer, shaders.mVolumetricParams1)) = Vector2::sZero;
    cbuffer.mSyncPending = true;
}

// Reconstructed from eboot.elf at 0x4561F0.
void RndVolumetricScatteringCom::_PostCreate() {
    RndAtmosphereCom::_PostCreate();
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x4564C0. _CreateTexture is inlined.
void RndVolumetricScatteringCom::_EditPoll() {
    if (mRuntimeData.mHeightDensityDirty) {
        mRuntimeData.mHeightDensityDirty = false;
        _CreateTexture();
    }
}

// Reconstructed from eboot.elf at 0x456530.
Symbol RndVolumetricScatteringCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x456540.
Symbol RndVolumetricScatteringCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x456550.
int RndVolumetricScatteringCom::CurrentRev() const {
    return const_cast<RndVolumetricScatteringCom*>(this)
        ->_GetPropRegistry()
        .mCurrentRev;
}

// Reconstructed from eboot.elf at 0x456570.
bool RndVolumetricScatteringCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x4565A0.
Component* RndVolumetricScatteringCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x4565B0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndVolumetricScatteringCom::_Imprint(
    char* buffer,
    Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndVolumetricScatteringCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndVolumetricScatteringCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x4566C0.
PropRegistry& RndVolumetricScatteringCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x4566D0.
ComMetaData& RndVolumetricScatteringCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x456770.
RndVolumetricScatteringCom::RndVolumetricScatteringCom(
    const RndVolumetricScatteringCom& other)
    : RndAtmosphereCom(other),
      mFogDensity(other.mFogDensity),
      mUseCameraEndDist(other.mUseCameraEndDist),
      mHeightRangeBegin(other.mHeightRangeBegin),
      mHeightRangeEnd(other.mHeightRangeEnd),
      mHeightDensity{
          ResourcePtr<Resource>(other.mHeightDensity.mResource.Get()),
          {0.0F, 0.0F},
          nullptr,
          ResourcePtr<Resource>(other.mHeightDensity.mOverride.Get()),
          {other.mHeightDensity.mOverrideData[0],
           other.mHeightDensity.mOverrideData[1]}},
      mVolumetricLightIntensity(other.mVolumetricLightIntensity),
      mRuntimeData{0.0F, false, ResourcePtr<RndTexture1DResource>()} {
    mQualitySettings._Copy(other.mQualitySettings);
    DrawPhases(mHeightDensity);
}
