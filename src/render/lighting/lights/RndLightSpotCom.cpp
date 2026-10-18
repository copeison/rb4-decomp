// render/RndLightSpotCom.o (0x49F2E0-0x4A867F). The registry's tag
// functions (0x4A7510 to 0x4A8610) are the lambdas in _Init, and the
// element type's registration (0x4A2120) is RegisterPropStruct. The
// PropArray element operations of "quality_settings" (vtable 0x19079A0)
// and the static initializer are not reconstructed.
#include "render/lighting/lights/RndLightSpotCom.h"

#include <cmath>
#include <functional>
#include <new>

#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "math/scalar/Trig.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector4.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/lighting/RndLightGlobals.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTexture2DResource.h"

namespace {

constexpr float kPi = 3.1415927F;
constexpr float kHalfPi = 1.5707964F;

// The square root the binary computes with a reciprocal square root and
// one Newton step, zero for zero.
float Length(float lengthSquared) {
    return lengthSquared == 0.0F ? 0.0F : std::sqrt(lengthSquared);
}

// The inputs of "intensity_calculator", which has no storage in the
// object; the beam angle is kept in radians. Names not in the reference
// map.
float sCalculatorLuminousFlux = 1100.0F;  // 0x19BDC38
float sCalculatorTemperature = 2500.0F;   // 0x19BDC3C
float sCalculatorBeamAngle = 0.7853982F;  // 0x19BDC40

// Whether the light casts shadows at some quality level, and whether it
// softens them at one. Inlined into the read-only tests of the shadow
// settings. Names not in the reference map.
bool CastsShadowsAtAnyLevel(const RndLightSpotCom& light) {
    if (!light.mCastsShadows) {
        return false;
    }
    const PropArray<RndLightSpotCom::ShadowQualitySettings>& levels = light.mQualitySettings;
    return levels[0].mCastsShadows || levels[1].mCastsShadows || levels[2].mCastsShadows;
}
bool SoftensShadowsAtAnyLevel(const RndLightSpotCom& light) {
    if (!light.mCastsShadows) {
        return false;
    }
    const PropArray<RndLightSpotCom::ShadowQualitySettings>& levels = light.mQualitySettings;
    return (levels[0].mCastsShadows && levels[0].mSoftenShadows) ||
        (levels[1].mCastsShadows && levels[1].mSoftenShadows) ||
        (levels[2].mCastsShadows && levels[2].mSoftenShadows);
}

// The choices of "falloff_function" and "angle_falloff_function". Inlined
// into _Init. Name not in the reference map.
eastl::vector<AllowedValue<int>> LightFalloffFunctions() {
    return eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            0,
            String("Default"),
            String("Use the default falloff function (Inverse Square for positive lights, Sigmoid for negative "
                   "lights)")},
        AllowedValue<int>{1, String("Linear"), String("Use a linear falloff")},
        AllowedValue<int>{
            2,
            String("Inverse Square"),
            String("Use an inverse-squared falloff approximation which goes to zero (the default for positive "
                   "lights, for which this is more or less physically correct)")},
        AllowedValue<int>{
            3,
            String("Exponential"),
            String("Use an inverse-exponential falloff approximation which goes to zero")},
        AllowedValue<int>{
            4,
            String("Sigmoid"),
            String("Use a sigmoid falloff approximation which goes to zero (the default for negative lights)")}};
}

// Reads a property of the component that holds the properties as they were
// saved into `value`; a missing or unconvertible property keeps the value.
// The release build keeps only the arguments of the error it reports.
// Inlined into the revision handler. Name not in the reference map.
template <typename T>
void ReadSavedProp(const Component& saved, const PropPath& path, T& value, PropertyType type) {
    void* storage = const_cast<Component*>(&saved);
    const PropInfo* const info = const_cast<Component&>(saved)._GetPropRegistry().FindProp(path, &storage);
    if (info != nullptr && _GetProp(saved, path, *info, storage, &value, type)) {
        return;
    }
    static_cast<void>(saved.MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
}

// The property callbacks _Init installs (0x4A7030 to 0x4A7240). Names not
// in the reference map.
void OnFalloffStartChanged(const PropAccessorArgs& args);
void OnFalloffEndChanged(const PropAccessorArgs& args);
float GetStartAngleDegrees(const PropAccessorArgs& args);
void SetStartAngleDegrees(const PropAccessorArgs& args);
float GetEndAngleDegrees(const PropAccessorArgs& args);
void SetEndAngleDegrees(const PropAccessorArgs& args);
void OnTruncationChanged(const PropAccessorArgs& args);
void OnCastsShadowsChanged(const PropAccessorArgs& args);
float GetLuminousFlux(const PropAccessorArgs& args);
void SetLuminousFlux(const PropAccessorArgs& args);
float GetTemperature(const PropAccessorArgs& args);
void SetTemperature(const PropAccessorArgs& args);
float GetBeamAngleDegrees(const PropAccessorArgs& args);
void SetBeamAngleDegrees(const PropAccessorArgs& args);

}  // namespace

Symbol RndLightSpotCom::sId("LightSpot");
Symbol RndLightSpotCom::sClassName("LightSpot");
PropRegistry RndLightSpotCom::sPropRegistry;
ComMetaData RndLightSpotCom::sMetaData;

// Reconstructed from eboot.elf at 0x49F2E0. The RuntimeData constructor is
// inlined here.
RndLightSpotCom::RndLightSpotCom()
    : mBulbRadius(0.05F),
      mFalloffStart(0.0F),
      mFalloffEnd(20.0F),
      mFalloffFunction(0),
      mStartAngle(0.0F),
      mEndAngle(0.7853982F),
      mAngleFalloffFunction(0),
      mTruncation(0.0F),
      mCastsShadows(false),
      mOnlyFlaggedObjects(true),
      mCastContext(3),
      mShadowOffset(0.05F),
      mShadowSoftnessMin(0.1F),
      mShadowSoftnessMax(1.0F),
      mMaxShadowSoftnessDistance(5.0F),
      mCookieTiling{1.0F, 1.0F} {}

// Reconstructed from eboot.elf at 0x49F4A0.
RndLightSpotCom::RuntimeData::RuntimeData()
    : mGeometryDirty(false),
      mDeferredCBuffer(nullptr),
      mShadowGenCBuffer(nullptr),
      mWorldCapSphere(),
      mCameraInside(false),
      mUseGeoClipPlane(false),
      mShadowMapIndex(-1),
      mShadowContributionIndex(-1),
      mShadowCameraId{0xFFFFFFFFU},
      mShadowCamera(nullptr) {
    mGeoClipPlane.c = 0.0F;
}

// Reconstructed from eboot.elf at 0x49F550.
RndLightSpotCom::~RndLightSpotCom() {
    RndShaderCBuffer::SafeDelete(mRuntimeData.mDeferredCBuffer);
    RndShaderCBuffer::SafeDelete(mRuntimeData.mShadowGenCBuffer);
}

// Reconstructed from eboot.elf at 0x49F690.
void RndLightSpotCom::SetFalloffStart(float distance) {
    mFalloffStart = distance;
    const float end = distance > mFalloffEnd ? distance : mFalloffEnd;
    const bool changed = end != mFalloffEnd;
    mFalloffEnd = end;
    if (changed) {
        _SyncGeometry();
        mRuntimeData.mGeometryDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x49F6E0.
void RndLightSpotCom::_SyncFalloffStart() {
    const float end = mFalloffEnd > mFalloffStart ? mFalloffEnd : mFalloffStart;
    const bool changed = end != mFalloffEnd;
    mFalloffEnd = end;
    if (changed) {
        _SyncGeometry();
        mRuntimeData.mGeometryDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x49F720.
void RndLightSpotCom::SetFalloffEnd(float distance) {
    mFalloffEnd = distance;
    mFalloffStart = distance < mFalloffStart ? distance : mFalloffStart;
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x49F760.
void RndLightSpotCom::_SyncFalloffEnd() {
    mFalloffStart = mFalloffEnd < mFalloffStart ? mFalloffEnd : mFalloffStart;
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x49F7A0.
void RndLightSpotCom::SetStartAngle(float angle) {
    mStartAngle = angle;
    const float end = angle > mEndAngle ? angle : mEndAngle;
    const bool changed = end != mEndAngle;
    mEndAngle = end;
    if (changed) {
        _SyncGeometry();
        mRuntimeData.mGeometryDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x49F7F0.
void RndLightSpotCom::_SyncStartAngle() {
    const float end = mEndAngle > mStartAngle ? mEndAngle : mStartAngle;
    const bool changed = end != mEndAngle;
    mEndAngle = end;
    if (changed) {
        _SyncGeometry();
        mRuntimeData.mGeometryDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x49F830.
void RndLightSpotCom::SetEndAngle(float angle) {
    mEndAngle = angle;
    mStartAngle = angle < mStartAngle ? angle : mStartAngle;
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x49F870.
void RndLightSpotCom::_SyncEndAngle() {
    mStartAngle = mEndAngle < mStartAngle ? mEndAngle : mStartAngle;
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x49F8B0. A cosine range closer to zero
// than 0.0001 is pushed out to it.
Vector3 RndLightSpotCom::GetAngleFalloffParams(float scale) const {
    const float outerAngle = kHalfPi < scale * mRuntimeData.mCone.mAngle ? kHalfPi : scale * mRuntimeData.mCone.mAngle;
    const float innerAngle = kHalfPi < scale * 0.5F * mStartAngle ? kHalfPi : scale * 0.5F * mStartAngle;
    const float innerCosine = Sine(innerAngle + kHalfPi);
    const float outerCosine = Sine(kHalfPi + outerAngle);
    int function;
    if (mAngleFalloffFunction != 0) {
        function = mAngleFalloffFunction - 1;
    } else {
        function = mIlluminationType == 3 ? 3 : 1;
    }
    float range = outerCosine - innerCosine;
    const bool tooSmall = range < 0.0F ? range > -0.0001F : range < 0.0001F;
    if (tooSmall) {
        range = range < 0.0F ? -0.0001F : 0.0001F;
    }
    const float reciprocal = 1.0F / range;
    return Vector3{reciprocal, -(innerCosine * reciprocal), static_cast<float>(function)};
}

// Reconstructed from eboot.elf at 0x49F9A0. The angles are edited in
// degrees and kept in radians. The shadow settings are read-only while no
// quality level casts shadows. "intensity_calculator" has no storage: its
// inputs live in globals, and its action sets the intensity from them.
// Revision 1 clears "only_flagged_objects" for lights saved before it;
// revision 2 adds the three quality levels and copies the old
// "shadows.soften" into each.
void RndLightSpotCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<ShadowQualitySettings>(Symbol("ShadowQualitySettings"));
    RndLightCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Spot light";
    metadata.mAuthor = "Daniel Sproul";
    RndLightSpotCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            RndLightSpotCom* const light = static_cast<RndLightSpotCom*>(args.mComponent);
            if (args.mSavedRev <= 0) {
                light->mOnlyFlaggedObjects = false;
            } else if (args.mSavedRev != 1) {
                return;
            }
            light->mQualitySettings.Resize(3);
            PropPath::Node shadows;
            PropPath::Node soften;
            const PropPath path(shadows = Symbol("shadows"), soften = Symbol("soften"));
            bool softened = false;
            ReadSavedProp(*args.mSavedProps, path, softened, kPropertyBool);
            light->mQualitySettings[0].mSoftenShadows = softened;
            light->mQualitySettings[1].mSoftenShadows = softened;
            light->mQualitySettings[2].mSoftenShadows = softened;
        };
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry == &own) {
        own.SetCurrentRev(2);
        own.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, own);
    }

    FloatMetadata& bulbRadius = TypeSpecificMetadata(
        registry.RegisterProp("bulb_radius", offsetof(RndLightSpotCom, mBulbRadius), kPropertyFloat, 4),
        prototype.mBulbRadius);
    bulbRadius.mDefault = prototype.mBulbRadius;
    static_cast<void>(Symbol("prop"));
    bulbRadius.mHelp = String("Radius of the light bulb; mostly determines the size of the specular highlight");
    bulbRadius.mMin = 0.0F;
    bulbRadius.mMax = 1000.0F;
    bulbRadius.mSlider = true;
    bulbRadius.mSliderLogResolution = 2.0F;

    FloatMetadata& falloffStart = TypeSpecificMetadata(
        registry.RegisterProp("falloff_start", offsetof(RndLightSpotCom, mFalloffStart), kPropertyFloat, 4),
        prototype.mFalloffStart);
    falloffStart.mDefault = prototype.mFalloffStart;
    static_cast<void>(Symbol("prop"));
    falloffStart.mMin = 0.0F;
    falloffStart.mMax = 1000.0F;
    falloffStart.mSlider = true;
    falloffStart.mSliderLogResolution = 2.0F;
    falloffStart.mOnChanged = OnFalloffStartChanged;

    FloatMetadata& falloffEnd = TypeSpecificMetadata(
        registry.RegisterProp("falloff_end", offsetof(RndLightSpotCom, mFalloffEnd), kPropertyFloat, 4),
        prototype.mFalloffEnd);
    falloffEnd.mDefault = prototype.mFalloffEnd;
    static_cast<void>(Symbol("prop"));
    falloffEnd.mMin = 0.0F;
    falloffEnd.mMax = 1000.0F;
    falloffEnd.mSlider = true;
    falloffEnd.mSliderLogResolution = 2.0F;
    falloffEnd.mOnChanged = OnFalloffEndChanged;

    IntMetadata<int>& falloffFunction = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "falloff_function",
        offsetof(RndLightSpotCom, mFalloffFunction), kPropertyInt, 4).mMetadata.Get());
    falloffFunction.mDefault = prototype.mFalloffFunction;
    static_cast<void>(Symbol("prop"));
    falloffFunction.mHelp = String(
        "Controls how light intensity falls off in distance as you move away from the light position");
    falloffFunction.mAllowedValues = LightFalloffFunctions();

    FloatMetadata& startAngle = TypeSpecificMetadata(
        registry.RegisterProp("start_angle", offsetof(RndLightSpotCom, mStartAngle), kPropertyFloat, 4),
        prototype.mStartAngle);
    startAngle.mDefault = prototype.mStartAngle;
    static_cast<void>(Symbol("prop"));
    startAngle.mHelp = String("Angle at which angular falloff begins");
    startAngle.mGetter = GetStartAngleDegrees;
    startAngle.mSetter = SetStartAngleDegrees;
    startAngle.mMin = 0.0F;
    startAngle.mMax = 179.99998F;
    startAngle.mSlider = true;
    startAngle.mUnits = Symbol("degrees");

    FloatMetadata& endAngle = TypeSpecificMetadata(
        registry.RegisterProp("end_angle", offsetof(RndLightSpotCom, mEndAngle), kPropertyFloat, 4),
        prototype.mEndAngle);
    endAngle.mDefault = prototype.mEndAngle;
    static_cast<void>(Symbol("prop"));
    endAngle.mHelp = String("Angle at which angular falloff ends");
    endAngle.mGetter = GetEndAngleDegrees;
    endAngle.mSetter = SetEndAngleDegrees;
    endAngle.mMin = 0.0F;
    endAngle.mMax = 179.99998F;
    endAngle.mSlider = true;
    endAngle.mUnits = Symbol("degrees");

    IntMetadata<int>& angleFalloff = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "angle_falloff_function",
        offsetof(RndLightSpotCom, mAngleFalloffFunction), kPropertyInt, 4).mMetadata.Get());
    angleFalloff.mDefault = prototype.mAngleFalloffFunction;
    static_cast<void>(Symbol("prop"));
    angleFalloff.mHelp = String(
        "Controls how light intensity falls off in angle as you move away from the light's central axis");
    angleFalloff.mAllowedValues = LightFalloffFunctions();

    FloatMetadata& truncation = TypeSpecificMetadata(
        registry.RegisterProp("truncation", offsetof(RndLightSpotCom, mTruncation), kPropertyFloat, 4),
        prototype.mTruncation);
    truncation.mDefault = prototype.mTruncation;
    static_cast<void>(Symbol("prop"));
    truncation.mHelp = String("Truncation of the spotlight falloff cone");
    truncation.mMin = 0.0F;
    truncation.mMax = 1.0F;
    truncation.mSlider = true;
    truncation.mSliderLogResolution = 0.001F;
    truncation.mOnChanged = OnTruncationChanged;

    PropRegistry& shadows =
        static_cast<GroupMetadata&>(*registry.RegisterProp("shadows", 0, kPropertyStruct, 0).mMetadata.Get())
            .mRegistry;
    static_cast<void>(Symbol("struct"));

    BoolMetadata& castsShadows = TypeSpecificMetadata(
        shadows.RegisterProp("casts_shadows", offsetof(RndLightSpotCom, mCastsShadows), kPropertyBool, 1),
        prototype.mCastsShadows);
    castsShadows.mDefault = prototype.mCastsShadows;
    static_cast<void>(Symbol("prop"));
    castsShadows.mHelp = String("Does this light cast shadows?");
    castsShadows.mOnChanged = OnCastsShadowsChanged;

    PropInfo& quality = shadows.RegisterPropArray(
        "quality_settings",
        offsetof(RndLightSpotCom, mQualitySettings),
        sizeof(PropArray<ShadowQualitySettings>));
    ArrayMetadata& qualityMetadata = static_cast<ArrayMetadata&>(*quality.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    qualityMetadata.mFixedSize = true;
    qualityMetadata.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightSpotCom&>(args.mComponent).mCastsShadows;
    });
    PropRegistry::RegisterPropArrayItem(quality, kPropertyStruct, sizeof(ShadowQualitySettings));
    GroupMetadata& level = static_cast<GroupMetadata&>(*qualityMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const ShadowQualitySettings levelPrototype = {true, true};
    level.mSummary._Set([](const TagArgs& args) {
        return String(RndQualityLevelDisplayName(static_cast<RndQualityLevel>(args.mPath[-1].mValue)));
    });
    PropRegistry& levelRegistry = level.mRegistry;

    BoolMetadata& levelCasts = TypeSpecificMetadata(
        levelRegistry.RegisterProp(
            "casts_shadows", offsetof(ShadowQualitySettings, mCastsShadows), kPropertyBool, 1),
        levelPrototype.mCastsShadows);
    levelCasts.mDefault = levelPrototype.mCastsShadows;
    static_cast<void>(Symbol("prop"));
    levelCasts.mHelp = String("Should this light cast shadows at this quality level?");

    BoolMetadata& levelSoften = TypeSpecificMetadata(
        levelRegistry.RegisterProp(
            "soften_shadows", offsetof(ShadowQualitySettings, mSoftenShadows), kPropertyBool, 1),
        levelPrototype.mSoftenShadows);
    levelSoften.mDefault = levelPrototype.mSoftenShadows;
    static_cast<void>(Symbol("prop"));
    levelSoften.mHelp = String("Should shadows be softened at this quality level?");
    levelSoften.mReadOnly._Set([](const TagArgs& args) {
        const RndLightSpotCom& light = static_cast<const RndLightSpotCom&>(args.mComponent);
        return !light.mQualitySettings[static_cast<int>(args.mPath[-2].mValue)].mCastsShadows;
    });

    BoolMetadata& onlyFlagged = TypeSpecificMetadata(
        shadows.RegisterProp(
            "only_flagged_objects", offsetof(RndLightSpotCom, mOnlyFlaggedObjects), kPropertyBool, 1),
        prototype.mOnlyFlaggedObjects);
    onlyFlagged.mDefault = prototype.mOnlyFlaggedObjects;
    static_cast<void>(Symbol("prop"));
    onlyFlagged.mHelp = String("Are shadows cast only from objects specifically tagged to do so?");
    onlyFlagged.mReadOnly._Set([](const TagArgs& args) {
        return !CastsShadowsAtAnyLevel(static_cast<const RndLightSpotCom&>(args.mComponent));
    });

    IntMetadata<unsigned int>& castContext = TypeSpecificMetadata(
        shadows.RegisterProp("cast_context", offsetof(RndLightSpotCom, mCastContext), kPropertyUInt, 4),
        prototype.mCastContext);
    castContext.mDefault = prototype.mCastContext;
    static_cast<void>(Symbol("prop"));
    castContext.mBitfield = true;
    castContext.mAllowedValues = eastl::vector<AllowedValue<unsigned int>>{
        AllowedValue<unsigned int>{1, String("Volumetric"), String("Shadows affect volumetrics.")},
        AllowedValue<unsigned int>{2, String("Geometry"), String("Shadows are cast on geometry.")}};
    castContext.mReadOnly._Set([](const TagArgs& args) {
        return !CastsShadowsAtAnyLevel(static_cast<const RndLightSpotCom&>(args.mComponent));
    });

    FloatMetadata& softnessMin = TypeSpecificMetadata(
        shadows.RegisterProp(
            "shadow_softness_min", offsetof(RndLightSpotCom, mShadowSoftnessMin), kPropertyFloat, 4),
        prototype.mShadowSoftnessMin);
    softnessMin.mDefault = prototype.mShadowSoftnessMin;
    static_cast<void>(Symbol("prop"));
    softnessMin.mHelp = String("Minimum softness of shadows when nearest to occluder.");
    softnessMin.mReadOnly._Set([](const TagArgs& args) {
        return !SoftensShadowsAtAnyLevel(static_cast<const RndLightSpotCom&>(args.mComponent));
    });
    softnessMin.mMin = 0.0F;
    softnessMin.mMax = 5.0F;
    softnessMin.mSlider = true;

    FloatMetadata& softnessMax = TypeSpecificMetadata(
        shadows.RegisterProp(
            "shadow_softness_max", offsetof(RndLightSpotCom, mShadowSoftnessMax), kPropertyFloat, 4),
        prototype.mShadowSoftnessMax);
    softnessMax.mDefault = prototype.mShadowSoftnessMax;
    static_cast<void>(Symbol("prop"));
    softnessMax.mHelp = String(
        "Maximum softness of shadows when further than max_shadow_softness_distance from occluder.");
    softnessMax.mReadOnly._Set([](const TagArgs& args) {
        return !SoftensShadowsAtAnyLevel(static_cast<const RndLightSpotCom&>(args.mComponent));
    });
    softnessMax.mMin = 0.0F;
    softnessMax.mMax = 5.0F;
    softnessMax.mSlider = true;

    FloatMetadata& softnessDistance = TypeSpecificMetadata(
        shadows.RegisterProp(
            "max_shadow_softness_distance",
            offsetof(RndLightSpotCom, mMaxShadowSoftnessDistance),
            kPropertyFloat,
            4),
        prototype.mMaxShadowSoftnessDistance);
    softnessDistance.mDefault = prototype.mMaxShadowSoftnessDistance;
    static_cast<void>(Symbol("prop"));
    softnessDistance.mHelp = String("Distance from occluder at which shadows are at max softness.");
    softnessDistance.mReadOnly._Set([](const TagArgs& args) {
        return !SoftensShadowsAtAnyLevel(static_cast<const RndLightSpotCom&>(args.mComponent));
    });
    softnessDistance.mMin = 0.0F;
    softnessDistance.mMax = 20.0F;
    softnessDistance.mSlider = true;

    FloatMetadata& shadowOffset = TypeSpecificMetadata(
        shadows.RegisterProp("shadow_offset", offsetof(RndLightSpotCom, mShadowOffset), kPropertyFloat, 4),
        prototype.mShadowOffset);
    shadowOffset.mDefault = prototype.mShadowOffset;
    static_cast<void>(Symbol("prop"));
    shadowOffset.mHelp = String("Increase this offset to remove shadow acne artifacts.");
    shadowOffset.mReadOnly._Set([](const TagArgs& args) {
        return !CastsShadowsAtAnyLevel(static_cast<const RndLightSpotCom&>(args.mComponent));
    });
    shadowOffset.mMin = 0.0F;
    shadowOffset.mMax = 0.2F;
    shadowOffset.mSlider = true;

    ResourcePathMetadata& cookie = TypeSpecificMetadata(
        registry.RegisterProp("cookie", offsetof(RndLightSpotCom, mCookie), kPropertyResourcePath, 8),
        prototype.mCookie);
    cookie.mDefault = prototype.mCookie;
    static_cast<void>(Symbol("prop"));
    cookie.mResourceId = RndTexture2DResource::Id();

    GroupMetadata& tiling = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "cookie_tiling",
        offsetof(RndLightSpotCom, mCookieTiling), kPropertyStruct, 8).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    tiling.mHelp = String("How much to tile the projected cookie texture");
    tiling.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const RndLightSpotCom&>(args.mComponent).mCookie == ResourcePath();
    });
    // The members' defaults come from a zeroed (u, v) pair, not the
    // prototype.
    const float tilingPrototype[2] = {0.0F, 0.0F};
    FloatMetadata& tilingU =
        TypeSpecificMetadata(tiling.mRegistry.RegisterProp("u", 0, kPropertyFloat, 4), tilingPrototype[0]);
    tilingU.mDefault = tilingPrototype[0];
    static_cast<void>(Symbol("prop"));
    tilingU.mMin = 0.0F;
    tilingU.mMax = 100.0F;
    tilingU.mSlider = true;
    tilingU.mSliderLogResolution = 1.0F;
    FloatMetadata& tilingV = TypeSpecificMetadata(
        tiling.mRegistry.RegisterProp("v", sizeof(float), kPropertyFloat, 4),
        tilingPrototype[1]);
    tilingV.mDefault = tilingPrototype[1];
    static_cast<void>(Symbol("prop"));
    tilingV.mMin = 0.0F;
    tilingV.mMax = 100.0F;
    tilingV.mSlider = true;
    tilingV.mSliderLogResolution = 1.0F;

    GroupMetadata& calculator = static_cast<GroupMetadata&>(
        *registry.RegisterProp("intensity_calculator", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    calculator.mHelp = String(
        "Use to calculate spotlight intensity based on real-world spotlight bulb numbers (eg. PAR or MR bulbs)");
    PropRegistry& calculatorRegistry = calculator.mRegistry;

    FloatMetadata& flux =
        TypeSpecificMetadata(calculatorRegistry.RegisterProp("luminous_flux", -1, kPropertyFloat, 0), float());
    static_cast<void>(Symbol("prop"));
    flux.mHelp = String("Light bulb output, in lumens");
    flux.mGetter = GetLuminousFlux;
    flux.mSetter = SetLuminousFlux;
    flux.mMin = 0.0F;
    flux.mMax = 5000.0F;
    flux.mUnits = Symbol("lumens");
    flux.mSlider = true;
    flux.mSliderLogResolution = 50.0F;

    FloatMetadata& temperature =
        TypeSpecificMetadata(calculatorRegistry.RegisterProp("temperature", -1, kPropertyFloat, 0), float());
    static_cast<void>(Symbol("prop"));
    temperature.mHelp = String("Light bulb temperature, in Kelvins");
    temperature.mGetter = GetTemperature;
    temperature.mSetter = SetTemperature;
    temperature.mMin = 1000.0F;
    temperature.mMax = 20000.0F;
    temperature.mUnits = Symbol("Kelvins");
    temperature.mSlider = true;
    temperature.mSliderLogResolution = 100.0F;

    FloatMetadata& beamAngle =
        TypeSpecificMetadata(calculatorRegistry.RegisterProp("beam_angle", -1, kPropertyFloat, 0), float());
    static_cast<void>(Symbol("prop"));
    beamAngle.mHelp = String("Light bulb beam angle -- the tighter the beam, the greater the intensity");
    beamAngle.mGetter = GetBeamAngleDegrees;
    beamAngle.mSetter = SetBeamAngleDegrees;
    beamAngle.mMin = 0.0F;
    beamAngle.mMax = 180.0F;
    beamAngle.mUnits = Symbol("degrees");
    beamAngle.mSlider = true;

    // The beam's solid angle is that of a cone of the full beam angle.
    ActionMetadata& calculate =
        static_cast<ActionMetadata&>(*calculatorRegistry.RegisterAction("calculate").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    calculate.mAction = [](const TagArgs& args) {
        RndLightSpotCom& light =
            const_cast<RndLightSpotCom&>(static_cast<const RndLightSpotCom&>(args.mComponent));
        const float efficacy = CalcLuminousEfficacy(sCalculatorTemperature);
        const float flux = sCalculatorLuminousFlux;
        const float solidAngle = (1.0F - Sine(sCalculatorBeamAngle * 0.5F + kHalfPi)) * 6.2831855F;
        light.mIntensity = flux / ((solidAngle > 0.0001F ? solidAngle : 0.0001F) * efficacy);
    };
}

// Reconstructed from eboot.elf at 0x4A22D0.
void RndLightSpotCom::_PostCreate() {
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x4A22F0.
void RndLightSpotCom::_PreDestroy(DestroyType type) {
    RndLightCom::_PreDestroy(type);
    if (type != kDestroyComponent) {
        return;
    }
    RndDrawNodeCom* node = mObject->GetCom<RndDrawNodeCom>();
    if (node == nullptr) {
        return;
    }
    const Transform& xfm = mObject->GetExistingCom<TransCom>()->mWorldXfm;
    node->SetLocalSphere(Sphere{xfm.v, 0.0F});
}

// Reconstructed from eboot.elf at 0x4A28A0. The light globals' spotlight
// mesh has mSpotlightSegments sides around the axis and mSpotlightCapSegments
// rings on the cap; the mesh cone grows by the reciprocal cosines of their
// half steps so the flat facets stay outside the volume.
void RndLightSpotCom::_SyncGeometry() {
    const float angle = mEndAngle * 0.5F;
    float topRadius = 0.0F;
    if (angle > 0.0F) {
        const float cosine = Sine(angle + kHalfPi);
        topRadius = mFalloffEnd * Sine(angle) / (1.0F - cosine);
    }
    TruncatedRoundedCone& cone = mRuntimeData.mCone;
    cone.SetAngleTopRadiusAndLength(angle, topRadius * mTruncation, mFalloffEnd);

    const RndLightGlobals& lighting = TheRndDevice()->mLighting;
    const float sideStep = kPi / static_cast<float>(lighting.mSpotlightSegments);
    const float capStep = cone.mAngle / static_cast<float>(lighting.mSpotlightCapSegments);
    const float sideScale = 1.0F / Sine(sideStep + kHalfPi);
    const float scale = 1.0F / Sine(kHalfPi + capStep) * sideScale;
    const float length = (cone.mLength - cone.mConeLength) * scale + cone.mConeLength;
    mRuntimeData.mMeshCone.SetRadiiAndLength(scale * cone.mTopRadius, scale * cone.mBottomRadius, length);
}

// Reconstructed from eboot.elf at 0x4A2B20. The map's _SyncLocalSphere is
// inlined: the sphere through the mesh cone's top rim and its cap rim.
void RndLightSpotCom::_Poll() {
    RndLightCom::_Poll();
    if (mRuntimeData.mGeometryDirty) {
        const TruncatedRoundedCone& meshCone = mRuntimeData.mMeshCone;
        const float bottomSquared = meshCone.mBottomRadius * meshCone.mBottomRadius;
        float centerDepth = 0.0F;
        if (0.0F < meshCone.mConeLength) {
            const float depth =
                (meshCone.mConeLength * meshCone.mConeLength + bottomSquared -
                 meshCone.mTopRadius * meshCone.mTopRadius) /
                (meshCone.mConeLength + meshCone.mConeLength);
            centerDepth = meshCone.mConeLength < depth ? meshCone.mConeLength : depth;
        }
        const float rimDepth = meshCone.mConeLength - centerDepth;
        const float radius = Length(rimDepth * rimDepth + bottomSquared);
        mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(
            Sphere{Vector3{0.0F, 0.0F, -centerDepth}, radius});
        mRuntimeData.mGeometryDirty = false;
    }
    if ((mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags & 1) == 0 && IsOn()) {
        const Transform& xfm = mRuntime.mTrans->mWorldXfm;
        const Hmx::Matrix3& m = xfm.m;
        const float xScaleSquared = m.x.x * m.x.x + m.x.y * m.x.y + m.x.z * m.x.z;
        const float yScaleSquared = m.y.x * m.y.x + m.y.y * m.y.y + m.y.z * m.y.z;
        const float sideScale = Length(yScaleSquared > xScaleSquared ? yScaleSquared : xScaleSquared);
        const float zScaleSquared = m.z.x * m.z.x + m.z.y * m.z.y + m.z.z * m.z.z;
        const float zScale = Length(zScaleSquared);
        const float zInverse = 1.0F / std::sqrt(zScaleSquared);
        const TruncatedRoundedCone& cone = mRuntimeData.mCone;
        const float length = zScale * cone.mLength;
        const Vector3 end{
            xfm.v.x - m.z.x * zInverse * length,
            xfm.v.y - m.z.y * zInverse * length,
            xfm.v.z - m.z.z * zInverse * length};
        const float endRadius = sideScale * (cone.mCapRadius / cone.mCapRimHeight * cone.mBottomRadius);
        mRuntimeData.mWorldBounds.Set(xfm.v, end, sideScale * cone.mTopRadius, endRadius);
        const float capOffset = cone.mCapRadius - cone.mLength;
        mRuntimeData.mWorldCapSphere.center.x = capOffset * m.z.x + xfm.v.x;
        mRuntimeData.mWorldCapSphere.center.y = capOffset * m.z.y + xfm.v.y;
        mRuntimeData.mWorldCapSphere.center.z = capOffset * m.z.z + xfm.v.z;
        mRuntimeData.mWorldCapSphere.radius = cone.mCapRadius;
    }
    if (mCastsShadows && (mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags & 1) == 0 && IsOn()) {
        _SyncShadowPlanes();
    }
}

// Reconstructed from eboot.elf at 0x4A3050.
int RndLightSpotCom::_GetCapabilitiesImpl() const {
    return 3;
}

// Reconstructed from eboot.elf at 0x4A3060.
Symbol RndLightSpotCom::_GetIntensityUnitsImpl() const {
    return Symbol("W/sr");
}

// Reconstructed from eboot.elf at 0x4A4CD0.
bool RndLightSpotCom::_CastsShadowsImpl(RndQualityLevel quality) const {
    if (!mCastsShadows) {
        return false;
    }
    const auto* settings = static_cast<const ShadowQualitySettings*>(mQualitySettings.mData);
    return settings[static_cast<int>(quality)].mCastsShadows;
}

// Reconstructed from eboot.elf at 0x4A52B0.
bool RndLightSpotCom::_AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr) {
    if (!castsShadows) {
        mRuntimeData.mShadowMapIndex = -1;
        mRuntimeData.mShadowContributionIndex = -1;
        return false;
    }
    mRuntimeData.mShadowMapIndex = mgr.AcquireSpotShadowDepthLayers(1);
    long contribution = -1;
    if (mRuntimeData.mShadowMapIndex != -1) {
        contribution = mgr.AcquireShadowContribution();
    }
    mRuntimeData.mShadowContributionIndex = contribution;
    return contribution != -1 && mRuntimeData.mShadowMapIndex != -1;
}

// Reconstructed from eboot.elf at 0x4A6920.
bool RndLightSpotCom::_IsOnImpl() const {
    return std::fabs(mFalloffEnd) > 0.0001F && std::fabs(mEndAngle) > 0.0001F;
}

// Reconstructed from eboot.elf at 0x4A6960.
void RndLightSpotCom::_SyncTruncation() {
    _SyncGeometry();
    mRuntimeData.mGeometryDirty = true;
}

// Reconstructed from eboot.elf at 0x4A6980. The depth range runs from the
// cap's rim to its end, at least 0.1 and 0.2.
void RndLightSpotCom::_CalcShadowParams(Vector4& projection, Vector4& depthRange) const {
    if (!mCastsShadows) {
        return;
    }
    const TruncatedRoundedCone& cone = mRuntimeData.mCone;
    const float farDepth = cone.mCapRadius > 0.2F ? cone.mCapRadius : 0.2F;
    const float rimDepth = cone.mCapRadius - cone.mLength;
    const float nearDepth = rimDepth > 0.1F ? rimDepth : 0.1F;
    depthRange.x = nearDepth;
    depthRange.y = farDepth;
    depthRange.z = nearDepth * farDepth;
    depthRange.w = farDepth - nearDepth;
    const float bottomRadius = cone.mBottomRadius > 0.0001F ? cone.mBottomRadius : 0.0001F;
    const float slope = cone.mCapRimHeight / bottomRadius;
    projection.x = slope * -0.5F;
    projection.y = slope * 0.5F;
    projection.z = 0.5F;
    projection.w = 0.5F;
}

// Reconstructed from eboot.elf at 0x4A6C50.
Symbol RndLightSpotCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x4A6C60.
Symbol RndLightSpotCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x4A6C70.
int RndLightSpotCom::CurrentRev() const {
    return const_cast<RndLightSpotCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x4A6C90.
bool RndLightSpotCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x4A6CC0.
Component* RndLightSpotCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x4A6CD0. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightSpotCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightSpotCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightSpotCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x4A6FA0.
PropRegistry& RndLightSpotCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x4A6FB0.
ComMetaData& RndLightSpotCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4A6FC0.
int RndLightSpotCom::_GetTypeImpl() const {
    return 1;
}

namespace {

// Reconstructed from eboot.elf at 0x4A7030.
void OnFalloffStartChanged(const PropAccessorArgs& args) {
    static_cast<RndLightSpotCom*>(args.mComponent)->_SyncFalloffStart();
}

// Reconstructed from eboot.elf at 0x4A7070.
void OnFalloffEndChanged(const PropAccessorArgs& args) {
    static_cast<RndLightSpotCom*>(args.mComponent)->_SyncFalloffEnd();
}

// Reconstructed from eboot.elf at 0x4A70B0.
float GetStartAngleDegrees(const PropAccessorArgs& args) {
    return static_cast<RndLightSpotCom*>(args.mComponent)->mStartAngle * 57.29578F;
}

// Reconstructed from eboot.elf at 0x4A70D0.
void SetStartAngleDegrees(const PropAccessorArgs& args) {
    RndLightSpotCom* const light = static_cast<RndLightSpotCom*>(args.mComponent);
    light->mStartAngle = *static_cast<const float*>(args.mValue) * 0.017453292F;
    light->_SyncStartAngle();
}

// Reconstructed from eboot.elf at 0x4A7130.
float GetEndAngleDegrees(const PropAccessorArgs& args) {
    return static_cast<RndLightSpotCom*>(args.mComponent)->mEndAngle * 57.29578F;
}

// Reconstructed from eboot.elf at 0x4A7150.
void SetEndAngleDegrees(const PropAccessorArgs& args) {
    RndLightSpotCom* const light = static_cast<RndLightSpotCom*>(args.mComponent);
    light->mEndAngle = *static_cast<const float*>(args.mValue) * 0.017453292F;
    light->_SyncEndAngle();
}

// Reconstructed from eboot.elf at 0x4A71B0.
void OnTruncationChanged(const PropAccessorArgs& args) {
    static_cast<RndLightSpotCom*>(args.mComponent)->_SyncTruncation();
}

// Reconstructed from eboot.elf at 0x4A71D0.
void OnCastsShadowsChanged(const PropAccessorArgs& args) {
    static_cast<RndLightSpotCom*>(args.mComponent)->_CreateShadowMapData();
}

// Reconstructed from eboot.elf at 0x4A71E0.
float GetLuminousFlux(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return sCalculatorLuminousFlux;
}

// Reconstructed from eboot.elf at 0x4A71F0.
void SetLuminousFlux(const PropAccessorArgs& args) {
    sCalculatorLuminousFlux = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x4A7200.
float GetTemperature(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return sCalculatorTemperature;
}

// Reconstructed from eboot.elf at 0x4A7210.
void SetTemperature(const PropAccessorArgs& args) {
    sCalculatorTemperature = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x4A7220.
float GetBeamAngleDegrees(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return sCalculatorBeamAngle * 57.29578F;
}

// Reconstructed from eboot.elf at 0x4A7240.
void SetBeamAngleDegrees(const PropAccessorArgs& args) {
    sCalculatorBeamAngle = *static_cast<const float*>(args.mValue) * 0.017453292F;
}

}  // namespace

// Reconstructed from eboot.elf at 0x404700. The binary emits the factory
// with the class's Init (0x3F08C0), before the renderer's components.
Component* RndLightSpotCom::_Create() {
    return new RndLightSpotCom();
}

// Reconstructed from eboot.elf at 0x3F08C0. The binary emits the registration
// in render/RndInit.o.
void RndLightSpotCom::Init() {
    InitComponentClass<RndLightSpotCom>();
}
