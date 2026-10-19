// render/RndVolumetricScatteringCom.o (0x453C00 to 0x45760F). The object
// also emits the templates of PropArray<QualitySettings> (0x453DE0,
// 0x4568E0-0x456B80), the class description's type registration
// (0x456000), the quality names (0x4561B0, 0x4561D0) and the registry's
// property callbacks (0x456740-0x457500), among them the revision upgrade
// (0x456C10) that moves the old "fog_start_dist" and "fog_end_dist" into
// the atmosphere's distances.
#include "render/lighting/volumetric/RndVolumetricScatteringCom.h"

#include <new>

#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
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
#include "render/textures/RndTextureUtl.h"
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
// density is baked at (read by _CreateTexture, 0x456460), and the waveform
// resource a new height density starts from, which _Init (0x454A70) makes a
// constant curve of one. Names not in the reference map.
const Vector2i gWaveformTextureSize = {128, 8};  // 0x1A76018
ResourcePtr<WaveformResource<float>> gDefaultHeightDensity;  // 0x1A76020

// The shaders' units of "fog_density". Name not in the reference map.
constexpr float kFogDensityScale = 0.2F;

// Reads a property of the component that holds the properties as they were
// saved into `value`; false when it is missing or cannot be converted.
// Inlined into the revision handler. Name not in the reference map.
template <typename T>
bool ReadSavedProp(Component& saved, const PropPath& path, T& value, PropertyType type) {
    void* storage = &saved;
    const PropInfo* const info = saved._GetPropRegistry().FindProp(path, &storage);
    return info != nullptr && _GetProp(saved, path, *info, storage, &value, type);
}

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

}  // namespace

// Reconstructed from eboot.elf at 0x453C00.
RndVolumetricScatteringCom::RndVolumetricScatteringCom()
    : mFogDensity(0.2F),
      mUseCameraEndDist(false),
      mHeightRangeBegin(0.0F),
      mHeightRangeEnd(5.0F),
      mHeightDensity(gDefaultHeightDensity.Get()),
      mVolumetricLightIntensity(1.0F),
      mRuntimeData{0.0F, false, ResourcePtr<RndTexture1DResource>()} {}

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

// Reconstructed from eboot.elf at 0x454A70. The quality settings are
// summarized by the level's display name, and a level's resolution is
// read-only while it is disabled; the end distance is read-only while the
// camera's far plane gives it. The revision 4 handler (0x456C10) moves the
// distances saved before revision 4 as "fog_start_dist" and "fog_end_dist"
// into the atmosphere's, has the fog end at the far plane before revision
// 2, and before revision 1 gives every quality level the saved
// "resolution". The binary emits the struct registration at 0x456000.
void RndVolumetricScatteringCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<QualitySettings>(Symbol("RndVolumetricScatteringCom::QualitySettings"));
    _InitAsSuperclass(registry, metadata);
    // The default height density: a baked curve through the keys (0, 1) and
    // (1, 1) with Hermite interpolation. The binary inlines the resource's
    // constructor, SetType(2) and SetCurve.
    gDefaultHeightDensity = new WaveformResource<float>();
    gDefaultHeightDensity->SetType(2);
    TKeyTrack<ValueTangent<float>> defaultCurve;
    defaultCurve.mInterpolation = KeyTrackBase::kInterpolationHermite;
    defaultCurve.AddKey(ValueTangent<float>{1.0F, 0.0F, 0.0F, true}, 0.0F, false, false);
    defaultCurve.AddKey(ValueTangent<float>{1.0F, 0.0F, 0.0F, true}, 1.0F, false, false);
    gDefaultHeightDensity->SetCurve(0, defaultCurve);
    metadata.mDescription = "Simulates volumetric light scattering throughout the scene volume";
    metadata.mAuthor = "Daniel Sproul";
    RndVolumetricScatteringCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 3) {
                return;
            }
            RndVolumetricScatteringCom* const scattering =
                static_cast<RndVolumetricScatteringCom*>(args.mComponent);
            Component& saved = *args.mSavedProps;
            float startDist;
            if (ReadSavedProp(saved, PropPath(Symbol("fog_start_dist")), startDist, kPropertyFloat)) {
                scattering->SetStartDist(startDist);
            }
            float endDist;
            if (ReadSavedProp(saved, PropPath(Symbol("fog_end_dist")), endDist, kPropertyFloat)) {
                scattering->SetEndDist(endDist);
            }
            if (args.mSavedRev > 1) {
                return;
            }
            scattering->mUseCameraEndDist = true;
            if (args.mSavedRev == 1) {
                return;
            }
            scattering->mQualitySettings.Resize(3);
            const PropPath path(Symbol("resolution"));
            int resolution = 0;
            if (!ReadSavedProp(saved, path, resolution, kPropertyInt)) {
                // The release build keeps only the arguments of the error it
                // reports.
                static_cast<void>(saved.MakeErrorName());
                char text[512] = {};
                path.ToString(text, sizeof(text));
            }
            scattering->mQualitySettings[0].mResolution = resolution;
            scattering->mQualitySettings[1].mResolution = resolution;
            scattering->mQualitySettings[2].mResolution = resolution;
        };
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry == &own) {
        own.SetCurrentRev(4);
        own.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, own);
    }

    PropInfo& quality = registry.RegisterPropArray(
        "quality_settings",
        offsetof(RndVolumetricScatteringCom, mQualitySettings),
        sizeof(PropArray<QualitySettings>));
    ArrayMetadata& qualityMetadata = static_cast<ArrayMetadata&>(*quality.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    qualityMetadata.mFixedSize = true;
    qualityMetadata.mAnimatable = false;
    PropRegistry::RegisterPropArrayItem(quality, kPropertyStruct, sizeof(QualitySettings));
    GroupMetadata& level = static_cast<GroupMetadata&>(*qualityMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const QualitySettings levelPrototype;
    level.mSummary._Set([](const TagArgs& args) {
        return String(RndQualityLevelDisplayName(static_cast<RndQualityLevel>(args.mPath[-1].mValue)));
    });
    PropRegistry& levelRegistry = level.mRegistry;

    BoolMetadata& levelEnabled = TypeSpecificMetadata(
        levelRegistry.RegisterProp("enabled", offsetof(QualitySettings, mEnabled), kPropertyBool, 1),
        levelPrototype.mEnabled);
    levelEnabled.mDefault = levelPrototype.mEnabled;
    static_cast<void>(Symbol("prop"));
    levelEnabled.mHelp = String("Is volumetric scattering enabled at this quality level?");

    PropInfo& resolutionInfo =
        levelRegistry.RegisterProp("resolution", offsetof(QualitySettings, mResolution), kPropertyInt, 4);
    IntMetadata<int>& resolution = static_cast<IntMetadata<int>&>(*resolutionInfo.mMetadata.Get());
    resolution.mDefault = levelPrototype.mResolution;
    static_cast<void>(Symbol("prop"));
    resolution.mHelp = String("Resolution at which volumetric scattering is simulated");
    {
        eastl::vector<AllowedValue<int>> resolutions;
        resolutions.reserve(3);
        resolutions.push_back(AllowedValue<int>{0, String("Low"), String("Uses 128 slices in depth")});
        resolutions.push_back(AllowedValue<int>{1, String("Medium"), String("Uses 256 slices in depth")});
        resolutions.push_back(AllowedValue<int>{2, String("High"), String("Uses 512 slices in depth")});
        resolution.mAllowedValues = resolutions;
    }
    resolution.mReadOnly._Set([](const TagArgs& args) {
        const RndVolumetricScatteringCom& scattering =
            static_cast<const RndVolumetricScatteringCom&>(args.mComponent);
        return !scattering.mQualitySettings[static_cast<int>(args.mPath[-2].mValue)].mEnabled;
    });

    static_cast<void>(registry.FindProp(PropPath(Symbol("end_dist")), nullptr));
    const float distType = 0.0F;
    FloatMetadata& endDist = TypeSpecificMetadata(
        *const_cast<PropInfo*>(registry.FindProp(PropPath(Symbol("end_dist")), nullptr)), distType);
    static_cast<void>(Symbol("prop"));
    endDist.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndVolumetricScatteringCom&>(args.mComponent).mUseCameraEndDist;
    });

    BoolMetadata& useCameraEndDist = TypeSpecificMetadata(
        registry.RegisterProp(
            "use_camera_end_dist", offsetof(RndVolumetricScatteringCom, mUseCameraEndDist), kPropertyBool, 1),
        prototype.mUseCameraEndDist);
    useCameraEndDist.mDefault = prototype.mUseCameraEndDist;
    static_cast<void>(Symbol("prop"));
    useCameraEndDist.mHelp = String("Use the camera far plane distance as the fog end distance?");

    FloatMetadata& fogDensity = TypeSpecificMetadata(
        registry.RegisterProp("fog_density", offsetof(RndVolumetricScatteringCom, mFogDensity), kPropertyFloat, 4),
        prototype.mFogDensity);
    fogDensity.mDefault = prototype.mFogDensity;
    static_cast<void>(Symbol("prop"));
    fogDensity.mHelp = String("Density of the fog");
    fogDensity.mMin = 0.0F;
    fogDensity.mMax = 1.0F;
    fogDensity.mSlider = true;
    fogDensity.mOnChanged = [](const PropAccessorArgs& args) {
        RndVolumetricScatteringCom* const scattering = static_cast<RndVolumetricScatteringCom*>(args.mComponent);
        scattering->mRuntimeData.mFogDensity = scattering->mFogDensity * kFogDensityScale;
    };

    WaveformMetadata<float>& heightDensity = TypeSpecificMetadata(
        registry.RegisterProp(
            "height_density",
            offsetof(RndVolumetricScatteringCom, mHeightDensity),
            kPropertyWaveformFloat,
            sizeof(Waveform<float>)),
        prototype.mHeightDensity);
    heightDensity.mDefault = Waveform<float>(prototype.mHeightDensity);
    static_cast<void>(Symbol("prop"));
    heightDensity.mHelp = String("Specifies the density of the for over the specified height range");
    heightDensity.mAllowedTypes = {0, 1, 2};
    heightDensity.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<RndVolumetricScatteringCom*>(args.mComponent)->mRuntimeData.mHeightDensityDirty = true;
    };

    FloatMetadata& rangeBegin = TypeSpecificMetadata(
        registry.RegisterProp(
            "height_range_begin", offsetof(RndVolumetricScatteringCom, mHeightRangeBegin), kPropertyFloat, 4),
        prototype.mHeightRangeBegin);
    rangeBegin.mDefault = prototype.mHeightRangeBegin;
    static_cast<void>(Symbol("prop"));
    rangeBegin.mHelp = String("Beginning of the range for the density waveform");

    FloatMetadata& rangeEnd = TypeSpecificMetadata(
        registry.RegisterProp(
            "height_range_end", offsetof(RndVolumetricScatteringCom, mHeightRangeEnd), kPropertyFloat, 4),
        prototype.mHeightRangeEnd);
    rangeEnd.mDefault = prototype.mHeightRangeEnd;
    static_cast<void>(Symbol("prop"));
    rangeEnd.mHelp = String("End of the range for the density waveform");

    FloatMetadata& lightIntensity = TypeSpecificMetadata(
        registry.RegisterProp(
            "volumetric_light_intensity",
            offsetof(RndVolumetricScatteringCom, mVolumetricLightIntensity),
            kPropertyFloat,
            4),
        prototype.mVolumetricLightIntensity);
    lightIntensity.mDefault = prototype.mVolumetricLightIntensity;
    static_cast<void>(Symbol("prop"));
    lightIntensity.mHelp = String("Intensity of volumetric lighting");
    lightIntensity.mMin = 0.0F;
    lightIntensity.mMax = 3000.0F;
    lightIntensity.mSlider = true;
}

// Reconstructed from eboot.elf at 0x4561F0.
void RndVolumetricScatteringCom::_PostCreate() {
    RndAtmosphereCom::_PostCreate();
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x456460.
void RndVolumetricScatteringCom::_CreateTexture() {
    if (mHeightDensity.mResource->mType == 3) {
        return;
    }
    mRuntimeData.mHeightDensityTexture = RndTextureUtl::CreateWaveformFloatTextureResource(
        mHeightDensity, static_cast<RndTexWrapMode>(1), gWaveformTextureSize);
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
      mHeightDensity(other.mHeightDensity),
      mVolumetricLightIntensity(other.mVolumetricLightIntensity),
      mRuntimeData{0.0F, false, ResourcePtr<RndTexture1DResource>()} {
    mQualitySettings._Copy(other.mQualitySettings);
}

// Reconstructed from eboot.elf at 0x404890. The binary emits the factory
// with the class's Init (0x3F1BA0); the placement factory at 0x4048C0
// constructs in given storage.
Component* RndVolumetricScatteringCom::_Create() {
    return new RndVolumetricScatteringCom();
}

// Reconstructed from eboot.elf at 0x3F1BA0. The binary emits the
// registration in render/RndInit.o.
void RndVolumetricScatteringCom::Init() {
    InitComponentClass<RndVolumetricScatteringCom>();
}
