// render/RndLightPointCom.o (0x490060-0x496C9F). The registry's tag
// functions (0x495CA0 to 0x496C30) are the lambdas in _Init, and the
// element type's registration (0x492030) is RegisterPropStruct. The
// PropArray element operations of "quality_settings" (vtable 0x1906D50)
// and the static initializer are not reconstructed.
#include "render/lighting/lights/RndLightPointCom.h"

#include <cmath>
#include <functional>
#include <new>

#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "math/geometry/Sphere.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/lighting/RndLightGlobals.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/deferred/RndLightPointDeferredShader.h"
#include "render/lighting/shadows/RndShaderLightPointShadowGen.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndSceneResource.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureCube.h"
#include "render/textures/RndTextureCubeResource.h"

namespace {

// The inputs of "intensity_calculator", which has no storage in the
// object. Names not in the reference map.
float sCalculatorLuminousFlux = 1100.0F;  // 0x19BDC30
float sCalculatorTemperature = 2500.0F;   // 0x19BDC34

// Whether the light casts shadows at some quality level, and whether it
// softens them at one. Inlined into the read-only tests of the shadow
// settings. Names not in the reference map.
bool CastsShadowsAtAnyLevel(const RndLightPointCom& light) {
    if (!light.mCastsShadows) {
        return false;
    }
    const PropArray<RndLightPointCom::ShadowQualitySettings>& levels = light.mQualitySettings;
    return levels[0].mCastsShadows || levels[1].mCastsShadows || levels[2].mCastsShadows;
}
bool SoftensShadowsAtAnyLevel(const RndLightPointCom& light) {
    if (!light.mCastsShadows) {
        return false;
    }
    const PropArray<RndLightPointCom::ShadowQualitySettings>& levels = light.mQualitySettings;
    return (levels[0].mCastsShadows && levels[0].mSoftenShadows) ||
        (levels[1].mCastsShadows && levels[1].mSoftenShadows) ||
        (levels[2].mCastsShadows && levels[2].mSoftenShadows);
}

// The choices of "falloff_function". Inlined into _Init. Name not in the
// reference map.
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

// The property callbacks _Init installs (0x495930 to 0x4959E0). Names not
// in the reference map.
void OnFalloffStartChanged(const PropAccessorArgs& args);
void OnFalloffEndChanged(const PropAccessorArgs& args);
void OnCastsShadowsChanged(const PropAccessorArgs& args);
void OnShadowMapResolutionChanged(const PropAccessorArgs& args);
float GetLuminousFlux(const PropAccessorArgs& args);
void SetLuminousFlux(const PropAccessorArgs& args);
float GetTemperature(const PropAccessorArgs& args);
void SetTemperature(const PropAccessorArgs& args);

}  // namespace

Symbol RndLightPointCom::sId("LightPoint");
Symbol RndLightPointCom::sClassName("LightPoint");
PropRegistry RndLightPointCom::sPropRegistry;
ComMetaData RndLightPointCom::sMetaData;

// Reconstructed from eboot.elf at 0x490060. The RuntimeData constructor is
// inlined here.
RndLightPointCom::RndLightPointCom()
    : mBulbRadius(0.05F),
      mFalloffStart(0.0F),
      mFalloffEnd(20.0F),
      mFalloffFunction(0),
      mCastsShadows(false),
      mOnlyFlaggedObjects(true),
      mShadowMapResolution(1),
      mShadowOffset(0.05F),
      mShadowSoftnessMin(0.1F),
      mShadowSoftnessMax(1.0F),
      mMaxShadowSoftnessDistance(5.0F) {}

// Reconstructed from eboot.elf at 0x4901E0.
RndLightPointCom::~RndLightPointCom() {
    RndShaderCBuffer::SafeDelete(mRuntimeData.mDeferredCBuffer);
    RndShaderCBuffer::SafeDelete(mRuntimeData.mShadowGenCBuffer);
    delete mRuntimeData.mShadowMap;
    mRuntimeData.mShadowMap = nullptr;
}

// Reconstructed from eboot.elf at 0x490340. The shadow settings are
// read-only while no quality level casts shadows. "intensity_calculator"
// has no storage: its two inputs live in globals, and its action sets the
// intensity from them. Revision 1 clears "only_flagged_objects" for lights
// saved before it; revision 2 adds the three quality levels and copies the
// old "shadows.soften" into each.
void RndLightPointCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<ShadowQualitySettings>(Symbol("ShadowQualitySettings"));
    RndLightCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Point light";
    metadata.mAuthor = "Daniel Sproul";
    RndLightPointCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            RndLightPointCom* const light = static_cast<RndLightPointCom*>(args.mComponent);
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
        registry.RegisterProp("bulb_radius", offsetof(RndLightPointCom, mBulbRadius), kPropertyFloat, 4),
        prototype.mBulbRadius);
    bulbRadius.mDefault = prototype.mBulbRadius;
    static_cast<void>(Symbol("prop"));
    bulbRadius.mHelp = String("Radius of the light bulb; mostly determines the size of the specular highlight");
    bulbRadius.mMin = 0.0F;
    bulbRadius.mMax = 1000.0F;
    bulbRadius.mSlider = true;
    bulbRadius.mSliderLogResolution = 2.0F;

    FloatMetadata& falloffStart = TypeSpecificMetadata(
        registry.RegisterProp("falloff_start", offsetof(RndLightPointCom, mFalloffStart), kPropertyFloat, 4),
        prototype.mFalloffStart);
    falloffStart.mDefault = prototype.mFalloffStart;
    static_cast<void>(Symbol("prop"));
    falloffStart.mMin = 0.0F;
    falloffStart.mMax = 1000.0F;
    falloffStart.mSlider = true;
    falloffStart.mSliderLogResolution = 2.0F;
    falloffStart.mOnChanged = OnFalloffStartChanged;

    FloatMetadata& falloffEnd = TypeSpecificMetadata(
        registry.RegisterProp("falloff_end", offsetof(RndLightPointCom, mFalloffEnd), kPropertyFloat, 4),
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
        offsetof(RndLightPointCom, mFalloffFunction), kPropertyInt, 4).mMetadata.Get());
    falloffFunction.mDefault = prototype.mFalloffFunction;
    static_cast<void>(Symbol("prop"));
    falloffFunction.mHelp =
        String("Controls how light intensity falls off as you move away from the center of the light");
    falloffFunction.mAllowedValues = LightFalloffFunctions();

    PropRegistry& shadows =
        static_cast<GroupMetadata&>(*registry.RegisterProp("shadows", 0, kPropertyStruct, 0).mMetadata.Get())
            .mRegistry;
    static_cast<void>(Symbol("struct"));

    BoolMetadata& castsShadows = TypeSpecificMetadata(
        shadows.RegisterProp("casts_shadows", offsetof(RndLightPointCom, mCastsShadows), kPropertyBool, 1),
        prototype.mCastsShadows);
    castsShadows.mDefault = prototype.mCastsShadows;
    static_cast<void>(Symbol("prop"));
    castsShadows.mHelp = String("Does this light cast shadows?");
    castsShadows.mOnChanged = OnCastsShadowsChanged;

    PropInfo& quality = shadows.RegisterPropArray(
        "quality_settings",
        offsetof(RndLightPointCom, mQualitySettings),
        sizeof(PropArray<ShadowQualitySettings>));
    ArrayMetadata& qualityMetadata = static_cast<ArrayMetadata&>(*quality.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    qualityMetadata.mFixedSize = true;
    qualityMetadata.mAnimatable = false;
    qualityMetadata.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightPointCom&>(args.mComponent).mCastsShadows;
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
        const RndLightPointCom& light = static_cast<const RndLightPointCom&>(args.mComponent);
        return !light.mQualitySettings[static_cast<int>(args.mPath[-2].mValue)].mCastsShadows;
    });

    BoolMetadata& onlyFlagged = TypeSpecificMetadata(
        shadows.RegisterProp(
            "only_flagged_objects", offsetof(RndLightPointCom, mOnlyFlaggedObjects), kPropertyBool, 1),
        prototype.mOnlyFlaggedObjects);
    onlyFlagged.mDefault = prototype.mOnlyFlaggedObjects;
    static_cast<void>(Symbol("prop"));
    onlyFlagged.mHelp = String("Are shadows cast only from objects specifically tagged to do so?");
    onlyFlagged.mReadOnly._Set([](const TagArgs& args) {
        return !CastsShadowsAtAnyLevel(static_cast<const RndLightPointCom&>(args.mComponent));
    });

    FloatMetadata& softnessMin = TypeSpecificMetadata(
        shadows.RegisterProp(
            "shadow_softness_min", offsetof(RndLightPointCom, mShadowSoftnessMin), kPropertyFloat, 4),
        prototype.mShadowSoftnessMin);
    softnessMin.mDefault = prototype.mShadowSoftnessMin;
    static_cast<void>(Symbol("prop"));
    softnessMin.mHelp = String("Minimum softness of shadows when nearest to occluder.");
    softnessMin.mReadOnly._Set([](const TagArgs& args) {
        return !SoftensShadowsAtAnyLevel(static_cast<const RndLightPointCom&>(args.mComponent));
    });
    softnessMin.mMin = 0.0F;
    softnessMin.mMax = 5.0F;
    softnessMin.mSlider = true;

    FloatMetadata& softnessMax = TypeSpecificMetadata(
        shadows.RegisterProp(
            "shadow_softness_max", offsetof(RndLightPointCom, mShadowSoftnessMax), kPropertyFloat, 4),
        prototype.mShadowSoftnessMax);
    softnessMax.mDefault = prototype.mShadowSoftnessMax;
    static_cast<void>(Symbol("prop"));
    softnessMax.mHelp = String(
        "Maximum softness of shadows when further than max_shadow_softness_distance from occluder.");
    softnessMax.mReadOnly._Set([](const TagArgs& args) {
        return !SoftensShadowsAtAnyLevel(static_cast<const RndLightPointCom&>(args.mComponent));
    });
    softnessMax.mMin = 0.0F;
    softnessMax.mMax = 5.0F;
    softnessMax.mSlider = true;

    FloatMetadata& softnessDistance = TypeSpecificMetadata(
        shadows.RegisterProp(
            "max_shadow_softness_distance",
            offsetof(RndLightPointCom, mMaxShadowSoftnessDistance),
            kPropertyFloat,
            4),
        prototype.mMaxShadowSoftnessDistance);
    softnessDistance.mDefault = prototype.mMaxShadowSoftnessDistance;
    static_cast<void>(Symbol("prop"));
    softnessDistance.mHelp = String("Distance from occluder at which shadows are at max softness.");
    softnessDistance.mReadOnly._Set([](const TagArgs& args) {
        return !SoftensShadowsAtAnyLevel(static_cast<const RndLightPointCom&>(args.mComponent));
    });
    softnessDistance.mMin = 0.0F;
    softnessDistance.mMax = 20.0F;
    softnessDistance.mSlider = true;

    IntMetadata<int>& resolution = static_cast<IntMetadata<int>&>(*shadows.RegisterProp(
        "shadow_map_resolution",
        offsetof(RndLightPointCom, mShadowMapResolution), kPropertyInt, 4).mMetadata.Get());
    resolution.mDefault = prototype.mShadowMapResolution;
    static_cast<void>(Symbol("prop"));
    resolution.mHelp = String("Resolution of the shadow map");
    resolution.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const RndLightPointCom&>(args.mComponent).mCastsShadows;
    });
    resolution.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{0, String("256"), String("256x256 cube shadow map")},
        AllowedValue<int>{1, String("512"), String("512x512 cube shadow map")},
        AllowedValue<int>{2, String("1024"), String("1024x1024 cube shadow map")}};
    resolution.mOnChanged = OnShadowMapResolutionChanged;

    FloatMetadata& shadowOffset = TypeSpecificMetadata(
        shadows.RegisterProp("shadow_offset", offsetof(RndLightPointCom, mShadowOffset), kPropertyFloat, 4),
        prototype.mShadowOffset);
    shadowOffset.mDefault = prototype.mShadowOffset;
    static_cast<void>(Symbol("prop"));
    shadowOffset.mHelp = String("Increase this offset to remove shadow acne artifacts.");
    shadowOffset.mReadOnly._Set([](const TagArgs& args) {
        return !CastsShadowsAtAnyLevel(static_cast<const RndLightPointCom&>(args.mComponent));
    });
    shadowOffset.mMin = 0.0F;
    shadowOffset.mMax = 0.2F;
    shadowOffset.mSlider = true;

    ResourcePathMetadata& cookie = TypeSpecificMetadata(
        registry.RegisterProp("cookie", offsetof(RndLightPointCom, mCookie), kPropertyResourcePath, 8),
        prototype.mCookie);
    cookie.mDefault = prototype.mCookie;
    static_cast<void>(Symbol("prop"));
    cookie.mResourceId = RndTextureCubeResource::Id();

    GroupMetadata& calculator = static_cast<GroupMetadata&>(
        *registry.RegisterProp("intensity_calculator", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    calculator.mHelp = String("Use to calculate point light intensity based on real-world light bulb numbers");
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

    ActionMetadata& calculate =
        static_cast<ActionMetadata&>(*calculatorRegistry.RegisterAction("calculate").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    calculate.mAction = [](const TagArgs& args) {
        RndLightPointCom& light =
            const_cast<RndLightPointCom&>(static_cast<const RndLightPointCom&>(args.mComponent));
        const float efficacy = CalcLuminousEfficacy(sCalculatorTemperature);
        light.mIntensity = sCalculatorLuminousFlux / efficacy * 0.095492966F;
    };
}

// Reconstructed from eboot.elf at 0x4921E0.
void RndLightPointCom::_PostCreate() {
    mQualitySettings.Resize(3);
}

// Reconstructed from eboot.elf at 0x492200.
void RndLightPointCom::_PreDestroy(DestroyType type) {
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

// Reconstructed from eboot.elf at 0x4922E0. The cookie is loaded through
// Resource::GetOrLoad<RndTextureCubeResource>, which the binary emits in
// this object at 0x4924E0; the map's _SyncLocalSphere is inlined.
bool RndLightPointCom::_OnResourcesLoaded() {
    RndLightCom::_OnResourcesLoaded();
    RndLightGlobals& lighting = TheRndDevice()->mLighting;
    if (mRuntimeData.mDeferredCBuffer == nullptr) {
        mRuntimeData.mDeferredCBuffer = RndShaderCBuffer::New(*lighting.mPointShader->mCBufferConfig, 0);
    }
    if (mRuntimeData.mShadowGenCBuffer == nullptr) {
        mRuntimeData.mShadowGenCBuffer =
            RndShaderCBuffer::New(*lighting.mPointShadowGenShader->mCBufferConfig, 0);
    }
    const ResourcePtr<RndTextureCubeResource> previous(mRuntimeData.mCookieTexture.Get());
    if (mCookie == ResourcePath()) {
        mRuntimeData.mCookieTexture = nullptr;
    } else {
        mRuntimeData.mCookieTexture = Resource::GetOrLoad<RndTextureCubeResource>(mCookie, false);
    }
    if (previous.Get() != mRuntimeData.mCookieTexture.Get()) {
        _CookieTextureChanged();
    }
    _CreateShadowMapData();
    mRuntimeData.mSphereDirty = false;
    mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(Sphere{Vector3::sZero, mFalloffEnd});
    return true;
}

// Reconstructed from eboot.elf at 0x492920.
void RndLightPointCom::_Poll() {
    RndLightCom::_Poll();
    if (mRuntimeData.mSphereDirty) {
        mRuntimeData.mSphereDirty = false;
        mObject->GetExistingCom<RndDrawNodeCom>()->SetLocalSphere(Sphere{Vector3::sZero, mFalloffEnd});
    }
}

// Reconstructed from eboot.elf at 0x4929D0.
int RndLightPointCom::_GetCapabilitiesImpl() const {
    return 3;
}

// Reconstructed from eboot.elf at 0x4929E0.
Symbol RndLightPointCom::_GetIntensityUnitsImpl() const {
    return Symbol("W/sr");
}

// Reconstructed from eboot.elf at 0x493BE0.
RndTextureBase* RndLightPointCom::_GetCookieTextureImpl() const {
    RndTextureCubeResource* cookie = mRuntimeData.mCookieTexture.Get();
    if (cookie == nullptr || cookie->Fail()) {
        return nullptr;
    }
    return cookie->mTexture;
}

// Reconstructed from eboot.elf at 0x493C20.
bool RndLightPointCom::_CastsShadowsImpl(RndQualityLevel quality) const {
    static_cast<void>(quality);
    return false;
}

// Reconstructed from eboot.elf at 0x493FB0.
bool RndLightPointCom::_AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr) {
    long index = -1;
    if (castsShadows) {
        index = mgr.AcquireShadowContribution();
    }
    mRuntimeData.mShadowContributionIndex = index;
    return index != -1;
}

// Reconstructed from eboot.elf at 0x4954A0.
bool RndLightPointCom::_IsOnImpl() const {
    return std::fabs(mFalloffEnd) > 0.0001F;
}

// Reconstructed from eboot.elf at 0x4954C0.
void RndLightPointCom::_SyncFalloffStart() {
    const float end = mFalloffStart > mFalloffEnd ? mFalloffStart : mFalloffEnd;
    const bool changed = end != mFalloffEnd;
    mFalloffEnd = end;
    if (changed) {
        mRuntimeData.mSphereDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x4954F0.
void RndLightPointCom::_SyncFalloffEnd() {
    mFalloffStart = mFalloffEnd < mFalloffStart ? mFalloffEnd : mFalloffStart;
    mRuntimeData.mSphereDirty = true;
}

// Reconstructed from eboot.elf at 0x4955A0.
Symbol RndLightPointCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x4955B0.
Symbol RndLightPointCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x4955C0.
int RndLightPointCom::CurrentRev() const {
    return const_cast<RndLightPointCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x4955E0.
bool RndLightPointCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x495610.
Component* RndLightPointCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x495620. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightPointCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightPointCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightPointCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x4958A0.
PropRegistry& RndLightPointCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x4958B0.
ComMetaData& RndLightPointCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x4958C0.
int RndLightPointCom::_GetTypeImpl() const {
    return 0;
}

namespace {

// Reconstructed from eboot.elf at 0x495930.
void OnFalloffStartChanged(const PropAccessorArgs& args) {
    static_cast<RndLightPointCom*>(args.mComponent)->_SyncFalloffStart();
}

// Reconstructed from eboot.elf at 0x495960.
void OnFalloffEndChanged(const PropAccessorArgs& args) {
    static_cast<RndLightPointCom*>(args.mComponent)->_SyncFalloffEnd();
}

// Reconstructed from eboot.elf at 0x495990.
void OnCastsShadowsChanged(const PropAccessorArgs& args) {
    static_cast<RndLightPointCom*>(args.mComponent)->_CreateShadowMapData();
}

// Reconstructed from eboot.elf at 0x4959A0.
void OnShadowMapResolutionChanged(const PropAccessorArgs& args) {
    static_cast<RndLightPointCom*>(args.mComponent)->_CreateShadowMapData();
}

// Reconstructed from eboot.elf at 0x4959B0.
float GetLuminousFlux(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return sCalculatorLuminousFlux;
}

// Reconstructed from eboot.elf at 0x4959C0.
void SetLuminousFlux(const PropAccessorArgs& args) {
    sCalculatorLuminousFlux = *static_cast<const float*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x4959D0.
float GetTemperature(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return sCalculatorTemperature;
}

// Reconstructed from eboot.elf at 0x4959E0.
void SetTemperature(const PropAccessorArgs& args) {
    sCalculatorTemperature = *static_cast<const float*>(args.mValue);
}

}  // namespace

// Reconstructed from eboot.elf at 0x404660. The binary emits the factory
// with the class's Init (0x3F0120), before the renderer's components.
Component* RndLightPointCom::_Create() {
    return new RndLightPointCom();
}

// Reconstructed from eboot.elf at 0x3F0120. The binary emits the registration
// in render/RndInit.o.
void RndLightPointCom::Init() {
    InitComponentClass<RndLightPointCom>();
}
