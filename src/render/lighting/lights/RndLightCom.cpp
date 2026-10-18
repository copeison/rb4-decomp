// render/RndLightCom.o (0x46C950 to 0x4705FF). The registry's tag functions
// (0x46FAE0 to 0x470520) are the lambdas in _Init.
#include "render/lighting/lights/RndLightCom.h"

#include <cmath>
#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/InstanceCom.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRegistry.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/lights/RndLightEnvironCom.h"
#include "render/scene/RndDrawNodeCom.h"
#include "render/scene/RndDrawableEntityResource.h"
#include "render/scene/RndSceneCom.h"

namespace {

// The id of no object, at 0x1A873A0, which the static initializer (0x470530)
// sets with the shared header's thread-group widths. Each object that uses
// it has its own copy. Names not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};
[[maybe_unused]] int gLightComGroupSize2D = 8;  // 0x1A873A4
[[maybe_unused]] int gLightComGroupSize3D = 4;  // 0x1A873A8

// A color channel matches when it is within this of the reference.
constexpr float kColorEpsilon = 0.0001F;

// The object's instance component, found by its class symbol as
// GameObject::GetBaseCom does. Inlined into its users. Name not in the
// reference map.
Component* FindInstanceCom(const GameObject* object) {
    if (InstanceCom::sClassName == Symbol()) {
        return nullptr;
    }
    for (const GameObject::ComIndex& index : object->mComs) {
        if (index.mBaseId == InstanceCom::sClassName) {
            return index.mCom;
        }
    }
    return nullptr;
}

// The entity an instance component instances. Name not in the reference
// map.
Entity* GetInstancedEntity(const Component* instance) {
    return static_cast<const InstanceCom*>(instance)->mRuntime.mEntity;
}

// The scene object above the object: the first object, from the object up
// through the objects that instance its entity, whose entity's root holds a
// RndSceneCom. Inlined into its users. Name not in the reference map.
GameObject* FindSceneObject(GameObject* object, RndSceneCom** scene) {
    for (; object != nullptr; object = object->mEntity->mParentObject) {
        if (RndSceneCom* found = object->mEntity->GetRoot()->GetCom<RndSceneCom>()) {
            *scene = found;
            return object;
        }
    }
    return nullptr;
}

bool ColorMatches(const Hmx::Color& color, const Hmx::Color& reference) {
    return std::fabs(color.red - reference.red) < kColorEpsilon &&
        std::fabs(color.green - reference.green) < kColorEpsilon &&
        std::fabs(color.blue - reference.blue) < kColorEpsilon;
}

// The reciprocal of the length, or 0 for a zero vector.
float InverseLength(float x, float y, float z) {
    const float length = std::sqrt(x * x + y * y + z * z);
    return length != 0.0F ? 1.0F / length : 0.0F;
}

// The property callbacks _Init installs (0x46F7E0 to 0x46FAC0). Names not
// in the reference map.
void RemoveEnvironment(const PropAccessorArgs& args);
void OnEnvironmentChanged(const PropAccessorArgs& args);
void OnIlluminationTypeChanged(const PropAccessorArgs& args);
void OnClipPlaneChanged(const PropAccessorArgs& args);
bool GetIsOn(const PropAccessorArgs& args);
bool GetHasZeroColor(const PropAccessorArgs& args);
bool GetHasZeroIntensity(const PropAccessorArgs& args);

}  // namespace

// The object's statics, in the order of its static initializer (0x470530).
Symbol RndLightCom::sId("Light");
Symbol RndLightCom::sClassName("Light");
PropRegistry RndLightCom::sPropRegistry;
ComMetaData RndLightCom::sMetaData;

// Reconstructed from eboot.elf at 0x46C950.
RndLightCom::RndLightCom()
    : mEnabled(true),
      mColor(Hmx::Color::GetWhite()),
      mIntensity(1.0F),
      mIlluminationType(0),
      mLightWrap(1.0F),
      mClipPlaneObject(gNullObjectId),
      mVolumetric(false) {}

// Reconstructed from eboot.elf at 0x46CAC0. The capabilities are left
// unset until _OnResourcesLoaded.
RndLightCom::RuntimeData::RuntimeData()
    : mTrans(nullptr),
      mDrawNode(nullptr),
      mVisible(false),
      mEnvironBits(0),
      mInBookkeeping(false),
      mBookkeepingDirty(false),
      mClipPlaneDirty(false),
      mClipPlaneTrans(nullptr),
      mMasterIntensityMult(1.0F),
      mIsolate(false),
      mReserved(false),
      mTiledCookieIndex(static_cast<unsigned long>(-1)),
      mCookieTexArrayIndex(-1),
      mLightBufferIndex(-1) {
    mClipPlane.a = 0.0F;
    mClipPlane.b = 0.0F;
    mClipPlane.c = 0.0F;
    mClipPlane.d = 0.0F;
}

// Reconstructed from eboot.elf at 0x46CB40. The arrays release their
// storage.
RndLightCom::~RndLightCom() {}

// Reconstructed from eboot.elf at 0x46CD40.
void RndLightCom::AddNestedLightsToBookkeeping(Component* instance) {
    const Entity* entity = GetInstancedEntity(instance);
    if (entity == nullptr) {
        return;
    }
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (RndLightCom* light = object->GetBaseCom<RndLightCom>()) {
            light->_InitBookkeeping();
        }
        if (Component* nested = FindInstanceCom(object)) {
            AddNestedLightsToBookkeeping(nested);
        }
    }
}

// Reconstructed from eboot.elf at 0x46CE60. A light in a scene's own entity
// names environments of that entity; a light in an instanced entity uses
// the instancing object's "nested_light_environments", which name objects
// of the instancing entity.
void RndLightCom::_InitBookkeeping() {
    mRuntime.mInBookkeeping = true;
    if (RndLightMgrCom* mgr = _FindActiveLightMgr()) {
        mgr->AddLight(this);
    }
    mRuntime.mBookkeepingDirty = false;

    const PropArray<GameObjectId>* environments;
    const Entity* entity;
    if (_IsInScene()) {
        environments = &mEnvironments;
        entity = mObject->mEntity;
    } else {
        RndSceneCom* scene = nullptr;
        GameObject* sceneObject = FindSceneObject(mObject, &scene);
        if (sceneObject == nullptr) {
            return;
        }
        Component* instance = FindInstanceCom(sceneObject);
        if (instance == nullptr) {
            return;
        }
        RndDrawNodeCom* node = instance->mObject->GetCom<RndDrawNodeCom>();
        if (node == nullptr) {
            return;
        }
        environments = &node->mNestedLightEnvironments;
        entity = instance->mObject->mEntity;
    }

    mRuntime.mActiveEnvironments.Reserve(environments->size());
    for (unsigned int i = 0; i < environments->size(); ++i) {
        const GameObjectId id = (*environments)[i];
        if (id.mId == gNullObjectId.mId) {
            continue;
        }
        GameObject* object = entity->GetObject(id);
        if (object == nullptr) {
            continue;
        }
        if (RndLightEnvironCom* environ = object->GetCom<RndLightEnvironCom>()) {
            environ->LightAdded(this);
            mRuntime.mActiveEnvironments._Insert(mRuntime.mActiveEnvironments.size(), &id);
        }
    }
}

// Reconstructed from eboot.elf at 0x46D390.
void RndLightCom::RemoveNestedLightsFromBookkeeping(Component* instance) {
    const Entity* entity = GetInstancedEntity(instance);
    if (entity == nullptr) {
        return;
    }
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (RndLightCom* light = object->GetBaseCom<RndLightCom>()) {
            light->_RemoveFromBookkeeping();
        }
        if (Component* nested = FindInstanceCom(object)) {
            RemoveNestedLightsFromBookkeeping(nested);
        }
    }
}

// Reconstructed from eboot.elf at 0x46D4B0.
void RndLightCom::_RemoveFromBookkeeping() {
    if (!mRuntime.mInBookkeeping) {
        return;
    }
    mRuntime.mInBookkeeping = false;
    if (RndLightMgrCom* mgr = _FindActiveLightMgr()) {
        mgr->RemoveLight(this);
    }
    if (const Entity* entity = _FindEnvironmentEntity()) {
        for (unsigned int i = 0; i < mRuntime.mActiveEnvironments.size(); ++i) {
            const GameObjectId id = mRuntime.mActiveEnvironments[i];
            if (id.mId == gNullObjectId.mId) {
                continue;
            }
            GameObject* object = entity->GetObject(id);
            if (object == nullptr) {
                continue;
            }
            if (RndLightEnvironCom* environ = object->GetCom<RndLightEnvironCom>()) {
                environ->LightRemoved(this);
            }
        }
    }
    mRuntime.mActiveEnvironments.Resize(0);
    mRuntime.mEnvironBits = 0;
}

// Reconstructed from eboot.elf at 0x46D680.
Plane RndLightCom::GetLocalClipPlane() const {
    Plane plane;
    Multiply(mRuntime.mClipPlane, mRuntime.mDrawNode->mRuntime.mInvWorldXfm, plane);
    const float scale = InverseLength(plane.a, plane.b, plane.c);
    plane.a *= scale;
    plane.b *= scale;
    plane.c *= scale;
    plane.d *= scale;
    return plane;
}

// Reconstructed from eboot.elf at 0x46D760. Lights implement the "Light"
// interface and need a draw node. A subtracting light (illumination type 3)
// gets the narrower intensity range; the run-time fields at the end are
// shown read-only for debugging and not saved.
void RndLightCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mCategory = ComMetaData::kCategoryLight;
    metadata.mInterface = sClassName;
    metadata.mDescription = "Properties common to all lights, should not be directly creatable";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mEditorRestrictions = 0;
    metadata.mRequiredComponents.push_back(RndDrawNodeCom::sClassName);
    metadata.mAllowedResources.push_back(RndDrawableEntityResource::Id());
    RndLightCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    BoolMetadata& enabled = TypeSpecificMetadata(
        registry.RegisterProp("enabled", offsetof(RndLightCom, mEnabled), kPropertyBool, 1),
        prototype.mEnabled);
    enabled.mDefault = prototype.mEnabled;
    static_cast<void>(Symbol("prop"));

    BoolMetadata& isolate = TypeSpecificMetadata(
        registry.RegisterProp(
            "isolate",
            offsetof(RndLightCom, mRuntime) + offsetof(RuntimeData, mIsolate),
            kPropertyBool,
            1),
        prototype.mRuntime.mIsolate);
    isolate.mDefault = prototype.mRuntime.mIsolate;
    static_cast<void>(Symbol("prop"));
    isolate.mSaves = false;

    BoolMetadata& volumetric = TypeSpecificMetadata(
        registry.RegisterProp("volumetric", offsetof(RndLightCom, mVolumetric), kPropertyBool, 1),
        prototype.mVolumetric);
    volumetric.mDefault = prototype.mVolumetric;
    static_cast<void>(Symbol("prop"));
    volumetric.mHelp = String("Does this light contribute to volumetric scattering?");

    PropInfo& environments = registry.RegisterPropArray(
        "environments",
        offsetof(RndLightCom, mEnvironments),
        sizeof(PropArray<GameObjectId>));
    ArrayMetadata& environmentsMetadata = static_cast<ArrayMetadata&>(*environments.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    environmentsMetadata.mHelp = String("Which environments does this light contribute light to?");
    environmentsMetadata.mIsHidden._Set([](const TagArgs& args) {
        return args.mComponent.mObject->mEntity->GetRoot()->GetCom<RndSceneCom>() == nullptr;
    });
    environmentsMetadata.mHook2 = reinterpret_cast<void*>(RemoveEnvironment);
    PropRegistry::RegisterPropArrayItem(environments, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId environType = {static_cast<unsigned int>(-1)};
    GameObjectIdMetadata& environ = TypeSpecificMetadata(environmentsMetadata.mItemInfo, environType);
    static_cast<void>(Symbol("item"));
    environ.mRequiredComs = {RndLightEnvironCom::sId};
    environ.mAllowsDuplicates = false;
    environ.mAllowsNull = false;
    environ.mOnChanged = OnEnvironmentChanged;

    ColorMetadata& color = TypeSpecificMetadata(
        registry.RegisterProp("color", offsetof(RndLightCom, mColor), kPropertyColor, 16),
        prototype.mColor);
    color.mDefault = prototype.mColor;
    static_cast<void>(Symbol("prop"));

    FloatMetadata& intensity = TypeSpecificMetadata(
        registry.RegisterProp("intensity", offsetof(RndLightCom, mIntensity), kPropertyFloat, 4),
        prototype.mIntensity);
    intensity.mDefault = prototype.mIntensity;
    static_cast<void>(Symbol("prop"));
    intensity.mHelp = String(
        "Controls the overall brightness of the light (intensity 1 is roughly equivalent to a 75W incadescent "
        "lightbulb");
    intensity.mMin = 0.0F;
    intensity.mMax._Set([](const TagArgs& args) {
        return static_cast<const RndLightCom&>(args.mComponent).mIlluminationType == 3 ? 1.0F : 50.0F;
    });
    intensity.mUnits._Set([](const TagArgs& args) {
        const RndLightCom& light = static_cast<const RndLightCom&>(args.mComponent);
        if (light.mIlluminationType == 3) {
            return Symbol("");
        }
        return light._GetIntensityUnitsImpl();
    });
    intensity.mSlider = true;
    intensity.mSliderLogResolution._Set([](const TagArgs& args) {
        return static_cast<const RndLightCom&>(args.mComponent).mIlluminationType == 3 ? 0.01F : 0.5F;
    });

    IntMetadata<int>& illumination = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "illumination_type",
        offsetof(RndLightCom, mIlluminationType), kPropertyInt, 4).mMetadata.Get());
    illumination.mDefault = prototype.mIlluminationType;
    static_cast<void>(Symbol("prop"));
    illumination.mHelp = String("Controls what type of illumination this light provides");
    illumination.mAllowedValues._Set([](const TagArgs& args) {
        eastl::vector<AllowedValue<int>> types;
        types.push_back(AllowedValue<int>{
            0, String("Diffuse and Specular"), String("Contributes both diffuse and specular light")});
        types.push_back(AllowedValue<int>{1, String("Diffuse Only"), String("Contributes only diffuse light")});
        types.push_back(AllowedValue<int>{2, String("Specular Only"), String("Contributes only specular light")});
        if ((static_cast<const RndLightCom&>(args.mComponent).mRuntime.mCapabilities & 1) != 0) {
            types.push_back(
                AllowedValue<int>{3, String("Negative"), String("Darkens any nearby accumulated light")});
        }
        return types;
    });
    illumination.mOnChanged = OnIlluminationTypeChanged;

    FloatMetadata& lightWrap = TypeSpecificMetadata(
        registry.RegisterProp("light_wrap", offsetof(RndLightCom, mLightWrap), kPropertyFloat, 4),
        prototype.mLightWrap);
    lightWrap.mDefault = prototype.mLightWrap;
    static_cast<void>(Symbol("prop"));
    lightWrap.mHelp = String(
        "Controls how light wraps around a surface (1.0 is physically correct, falling to zero for surfaces "
        "perpendicular to the light; 2.0 falls to zero for surfaces pointing away from the light source; 4.0 "
        "blasts light equally from all directions)");
    lightWrap.mMin = 0.0F;
    lightWrap.mMax = 4.0F;
    lightWrap.mSlider = true;

    GameObjectIdMetadata& clipPlane = TypeSpecificMetadata(
        registry.RegisterProp("clip_plane", offsetof(RndLightCom, mClipPlaneObject), kPropertyGameObjectId, 4),
        prototype.mClipPlaneObject);
    clipPlane.mDefault = prototype.mClipPlaneObject;
    static_cast<void>(Symbol("prop"));
    clipPlane.mHelp = String("If specified, light will be clipped based on the object's Z axis");
    clipPlane.mIsHidden._Set([](const TagArgs& args) {
        return (static_cast<const RndLightCom&>(args.mComponent).mRuntime.mCapabilities & 2) == 0;
    });
    clipPlane.mOnChanged = OnClipPlaneChanged;

    IntMetadata<unsigned int>& environBits = TypeSpecificMetadata(
        registry.RegisterProp(
            "environ_bits",
            offsetof(RndLightCom, mRuntime) + offsetof(RuntimeData, mEnvironBits),
            kPropertyUInt,
            4),
        prototype.mRuntime.mEnvironBits);
    environBits.mDefault = prototype.mRuntime.mEnvironBits;
    static_cast<void>(Symbol("prop"));
    environBits.mHelp = String("Raw environments bitfield");
    environBits.mReadOnly = true;
    environBits.mDebug = true;
    environBits.mSaves = false;

    IntMetadata<unsigned long>& cookieIndex = TypeSpecificMetadata(
        registry.RegisterProp(
            "tiled_cookie_index",
            offsetof(RndLightCom, mRuntime) + offsetof(RuntimeData, mTiledCookieIndex),
            kPropertyUInt64,
            8),
        prototype.mRuntime.mTiledCookieIndex);
    cookieIndex.mDefault = prototype.mRuntime.mTiledCookieIndex;
    static_cast<void>(Symbol("prop"));
    cookieIndex.mHelp = String("Index of cookie texture in my lightmgr's cookie texarray");
    cookieIndex.mReadOnly = true;
    cookieIndex.mDebug = true;
    cookieIndex.mSaves = false;

    BoolMetadata& isOn = TypeSpecificMetadata(registry.RegisterProp("is_on", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    isOn.mReadOnly = true;
    isOn.mDebug = true;
    isOn.mSaves = false;
    isOn.mGetter = GetIsOn;

    BoolMetadata& zeroColor =
        TypeSpecificMetadata(registry.RegisterProp("has_zero_color", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    zeroColor.mReadOnly = true;
    zeroColor.mDebug = true;
    zeroColor.mSaves = false;
    zeroColor.mGetter = GetHasZeroColor;

    BoolMetadata& zeroIntensity =
        TypeSpecificMetadata(registry.RegisterProp("has_zero_intensity", -1, kPropertyBool, 0), false);
    static_cast<void>(Symbol("prop"));
    zeroIntensity.mReadOnly = true;
    zeroIntensity.mDebug = true;
    zeroIntensity.mSaves = false;
    zeroIntensity.mGetter = GetHasZeroIntensity;

    PropInfo& active = registry.RegisterPropArray(
        "active_environments",
        offsetof(RndLightCom, mRuntime) + offsetof(RuntimeData, mActiveEnvironments),
        sizeof(PropArray<GameObjectId>));
    ArrayMetadata& activeMetadata = static_cast<ArrayMetadata&>(*active.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    activeMetadata.mReadOnly = true;
    activeMetadata.mDebug = true;
    activeMetadata.mSaves = false;
    PropRegistry::RegisterPropArrayItem(active, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId activeType = {static_cast<unsigned int>(-1)};
    GameObjectIdMetadata& activeEnviron = TypeSpecificMetadata(activeMetadata.mItemInfo, activeType);
    static_cast<void>(Symbol("item"));
    activeEnviron.mEntity._Set([](const TagArgs& args) {
        return static_cast<const RndLightCom&>(args.mComponent)._FindEnvironmentEntity();
    });
}

// Reconstructed from eboot.elf at 0x46E910.
bool RndLightCom::_OnResourcesLoaded() {
    mRuntime.mCapabilities = _GetCapabilitiesImpl();
    mRuntime.mTrans = mObject->GetCom<TransCom>();
    mRuntime.mDrawNode = mObject->GetCom<RndDrawNodeCom>();
    _SyncClipPlaneTrans();
    return true;
}

// Reconstructed from eboot.elf at 0x46E9D0. The clip-plane object must
// have a TransCom.
void RndLightCom::_SyncClipPlaneTrans() {
    mRuntime.mClipPlaneDirty = false;
    if (mClipPlaneObject.mId == gNullObjectId.mId) {
        mRuntime.mClipPlaneTrans = nullptr;
        mRuntime.mClipPlane.a = 0.0F;
        mRuntime.mClipPlane.b = 0.0F;
        mRuntime.mClipPlane.c = 0.0F;
        mRuntime.mClipPlane.d = 0.0F;
        return;
    }
    mRuntime.mClipPlaneTrans = mObject->mEntity->GetObject(mClipPlaneObject)->GetCom<TransCom>();
    _SyncClipPlane();
}

// Reconstructed from eboot.elf at 0x46EB30.
bool RndLightCom::_AreResourcesReady(int pass) {
    static_cast<void>(pass);
    _InitBookkeeping();
    return true;
}

// Reconstructed from eboot.elf at 0x46EB40.
void RndLightCom::_PreDestroy(DestroyType type) {
    if (type >= kDestroyInstance) {
        _RemoveFromBookkeeping();
        return;
    }
    mRuntime.mInBookkeeping = false;
    mRuntime.mActiveEnvironments.Resize(0);
    mRuntime.mEnvironBits = 0;
}

// Reconstructed from eboot.elf at 0x46EB90.
void RndLightCom::_GetComponentOrderDeps(eastl::vector<Symbol>& follows, eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(RndDrawNodeCom::sClassName);
}

// Reconstructed from eboot.elf at 0x46EC50. Bit 2 of the TransCom's flags
// marks a changed world transform.
void RndLightCom::_Poll() {
    if (mRuntime.mClipPlaneDirty) {
        _SyncClipPlaneTrans();
    } else if (mRuntime.mClipPlaneTrans != nullptr && (mRuntime.mClipPlaneTrans->mDirtyFlags & 2) != 0) {
        _SyncClipPlane();
    }
    mRuntime.mLightBufferIndex = -1;
    mRuntime.mVisible = false;
    if (IsOn()) {
        mRuntime.mVisible = (mRuntime.mDrawNode->mRuntime.mWorldShowHideFlags & 1) == 0;
    }
    if (mRuntime.mBookkeepingDirty) {
        _RemoveFromBookkeeping();
        _InitBookkeeping();
    }
}

// Reconstructed from eboot.elf at 0x46ED80. The plane faces the object's z
// axis through its position.
void RndLightCom::_SyncClipPlane() {
    const Transform& xfm = mRuntime.mClipPlaneTrans->mWorldXfm;
    const Vector3& axis = xfm.m.z;
    const float scale = InverseLength(axis.x, axis.y, axis.z);
    Plane& plane = mRuntime.mClipPlane;
    plane.a = scale * axis.x;
    plane.b = scale * axis.y;
    plane.c = scale * axis.z;
    plane.d = -(plane.a * xfm.v.x + plane.b * xfm.v.y + plane.c * xfm.v.z);
}

// Reconstructed from eboot.elf at 0x46EE40.
bool RndLightCom::IsOn() const {
    if (!mEnabled || !mEntered || kColorEpsilon > mIntensity) {
        return false;
    }
    const Hmx::Color& noLight = mIlluminationType == 3 ? Hmx::Color::GetWhite() : Hmx::Color::GetBlack();
    if (ColorMatches(mColor, noLight)) {
        return false;
    }
    return _IsOnImpl();
}

// Reconstructed from eboot.elf at 0x46EFE0.
void RndLightCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x46EFF0.
void RndLightCom::_CookieTextureChanged() {
    if (!mRuntime.mInBookkeeping) {
        return;
    }
    if (RndLightMgrCom* mgr = _FindActiveLightMgr()) {
        mgr->mCookieTexArraysDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x46F090.
RndLightMgrCom* RndLightCom::_FindActiveLightMgr() const {
    RndSceneCom* scene = nullptr;
    if (FindSceneObject(mObject, &scene) == nullptr) {
        return nullptr;
    }
    return scene->GetLightMgr();
}

// Reconstructed from eboot.elf at 0x46F120.
bool RndLightCom::_IsInScene() const {
    return mObject->mEntity->GetRoot()->GetCom<RndSceneCom>() != nullptr;
}

// Reconstructed from eboot.elf at 0x46F180.
Component* RndLightCom::_FindInstanceInScene() const {
    RndSceneCom* scene = nullptr;
    GameObject* sceneObject = FindSceneObject(mObject, &scene);
    if (sceneObject == nullptr) {
        return nullptr;
    }
    return FindInstanceCom(sceneObject);
}

// Reconstructed from eboot.elf at 0x46F260.
Entity* RndLightCom::_FindEnvironmentEntity() const {
    if (_IsInScene()) {
        return mObject->mEntity;
    }
    const Component* instance = _FindInstanceInScene();
    return instance != nullptr ? instance->mObject->mEntity : nullptr;
}

// Reconstructed from eboot.elf at 0x46F3A0.
RndLightMgrCom* RndLightCom::_FindSiblingLightMgr() const {
    RndSceneCom* scene = mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
    return scene != nullptr ? scene->GetLightMgr() : nullptr;
}

// Reconstructed from eboot.elf at 0x46F410.
Symbol RndLightCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x46F420.
Symbol RndLightCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x46F430.
int RndLightCom::CurrentRev() const {
    return const_cast<RndLightCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x46F450.
bool RndLightCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x46F480.
Component* RndLightCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x46F490. The copy goes at the buffer's next
// 8-byte boundary, and its properties follow it.
char* RndLightCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndLightCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndLightCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x46F6B0.
PropRegistry& RndLightCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x46F6C0.
ComMetaData& RndLightCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x46F6D0.
int RndLightCom::_GetCapabilitiesImpl() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x46F6E0.
Symbol RndLightCom::_GetIntensityUnitsImpl() const {
    return Symbol("unknown units");
}

// Reconstructed from eboot.elf at 0x46F730.
bool RndLightCom::_FrustumExcludesImpl(const Frustum& frustum, const RndLightCullPlanes& planes) const {
    static_cast<void>(frustum);
    static_cast<void>(planes);
    return false;
}

// Reconstructed from eboot.elf at 0x46F740.
void RndLightCom::_PreDrawNoComputeImpl(RndContext& ctx, const RndLightProbeParams& probe) {
    static_cast<void>(ctx);
    static_cast<void>(probe);
}

// Reconstructed from eboot.elf at 0x46F750.
void RndLightCom::_DrawDeferredNoComputeImpl(
    RndContext& ctx,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    const RndLightProbeParams& probe) {
    static_cast<void>(ctx);
    static_cast<void>(buffers);
    static_cast<void>(target);
    static_cast<void>(probe);
}

// Reconstructed from eboot.elf at 0x46F760.
void RndLightCom::_AddToComputeBufferImpl(
    RndQualityLevel quality,
    const RndCameraContext& camera,
    RndComputeBuffer& buffer) {
    static_cast<void>(quality);
    static_cast<void>(camera);
    static_cast<void>(buffer);
}

// Reconstructed from eboot.elf at 0x46F770.
int RndLightCom::_GetTypeImpl() const {
    return -1;
}

// Reconstructed from eboot.elf at 0x46F780.
RndTextureBase* RndLightCom::_GetCookieTextureImpl() const {
    return nullptr;
}

// Reconstructed from eboot.elf at 0x46F790.
bool RndLightCom::_CastsShadowsImpl(RndQualityLevel quality) const {
    static_cast<void>(quality);
    return false;
}

// Reconstructed from eboot.elf at 0x46F7A0.
void RndLightCom::_DrawShadowMapImpl(
    RndContext& ctx,
    RndSceneDrawer& drawer,
    const RndCameraContext& camera,
    const RndShowHideContext& showHide,
    VectorAdapter<PodVector<RndDrawInstance>>& instances,
    VectorAdapter<PodVector<RndDrawInstance*>>& sortable,
    RndLightMgrCom& mgr) {
    static_cast<void>(ctx);
    static_cast<void>(drawer);
    static_cast<void>(camera);
    static_cast<void>(showHide);
    static_cast<void>(instances);
    static_cast<void>(sortable);
    static_cast<void>(mgr);
}

// Reconstructed from eboot.elf at 0x46F7B0.
bool RndLightCom::_AcquireDeferredShadowContributionResourcesImpl(bool castsShadows, RndLightMgrCom& mgr) {
    static_cast<void>(castsShadows);
    static_cast<void>(mgr);
    return false;
}

// Reconstructed from eboot.elf at 0x46F7C0.
void RndLightCom::_GenerateDeferredShadowContributionImpl(
    RndContext& ctx,
    RndBufferCollection& buffers,
    RndSceneDrawTarget& target,
    RndTextureBase* sceneMask,
    RndLightMgrCom& mgr) {
    static_cast<void>(ctx);
    static_cast<void>(buffers);
    static_cast<void>(target);
    static_cast<void>(sceneMask);
    static_cast<void>(mgr);
}

// Reconstructed from eboot.elf at 0x46F7D0.
bool RndLightCom::_IsOnImpl() const {
    return true;
}

namespace {

// Reconstructed from eboot.elf at 0x46F7E0. Removes the element the path
// names from "environments" in place of the array's own removal; dropping
// a set environment registers the light again.
void RemoveEnvironment(const PropAccessorArgs& args) {
    RndLightCom* const light = static_cast<RndLightCom*>(args.mComponent);
    const unsigned long index = (*static_cast<const PropPath*>(args.mPath))[-1].mValue;
    PropArray<GameObjectId>& environments = light->mEnvironments;
    if (environments[index].mId != gNullObjectId.mId) {
        light->mRuntime.mBookkeepingDirty = true;
    }
    environments.Destruct(1, environments.ElementAt(index));
    const unsigned int size = environments.size();
    if (static_cast<unsigned int>(index) < size - 1) {
        environments.Move(size - 1 - index, environments.ElementAt(index), environments.ElementAt(index + 1));
    }
    --environments.mSize;
}

// Reconstructed from eboot.elf at 0x46F8A0.
void OnEnvironmentChanged(const PropAccessorArgs& args) {
    static_cast<RndLightCom*>(args.mComponent)->mRuntime.mBookkeepingDirty = true;
}

// Reconstructed from eboot.elf at 0x46F8B0. A light that becomes
// subtracting keeps its intensity at most 1, and white, which would add
// nothing, turns black.
void OnIlluminationTypeChanged(const PropAccessorArgs& args) {
    RndLightCom* const light = static_cast<RndLightCom*>(args.mComponent);
    if (light->mIlluminationType != 3) {
        return;
    }
    light->mIntensity = 1.0F < light->mIntensity ? 1.0F : light->mIntensity;
    const Hmx::Color& white = Hmx::Color::GetWhite();
    const Hmx::Color& color = light->mColor;
    if (color.red == white.red && color.green == white.green && color.blue == white.blue &&
        color.alpha == white.alpha) {
        light->mColor = Hmx::Color::GetBlack();
    }
}

// Reconstructed from eboot.elf at 0x46F9B0.
void OnClipPlaneChanged(const PropAccessorArgs& args) {
    static_cast<RndLightCom*>(args.mComponent)->mRuntime.mClipPlaneDirty = true;
}

// Reconstructed from eboot.elf at 0x46F9C0.
bool GetIsOn(const PropAccessorArgs& args) {
    return static_cast<RndLightCom*>(args.mComponent)->IsOn();
}

// Reconstructed from eboot.elf at 0x46F9D0. The color that adds nothing,
// as IsOn tests it.
bool GetHasZeroColor(const PropAccessorArgs& args) {
    const RndLightCom* const light = static_cast<RndLightCom*>(args.mComponent);
    const Hmx::Color& noLight =
        light->mIlluminationType == 3 ? Hmx::Color::GetWhite() : Hmx::Color::GetBlack();
    return ColorMatches(light->mColor, noLight);
}

// Reconstructed from eboot.elf at 0x46FAC0.
bool GetHasZeroIntensity(const PropAccessorArgs& args) {
    return !(kColorEpsilon < static_cast<RndLightCom*>(args.mComponent)->mIntensity);
}

}  // namespace

// Reconstructed from eboot.elf at 0x4045C0. The binary emits the factory
// with the class's Init; the placement factory at 0x4045F0 constructs in
// given storage.
Component* RndLightCom::_Create() {
    return new RndLightCom();
}

// Reconstructed from eboot.elf at 0x3EF980. The binary emits the
// registration in render/RndInit.o. The class is a base that is not
// created itself.
void RndLightCom::Init() {
    InitComponentClass<RndLightCom>(false, true);
}
