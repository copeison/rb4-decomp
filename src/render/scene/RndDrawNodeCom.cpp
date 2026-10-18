// render/RndDrawNodeCom.o (0x6C3D40 to 0x6C944F). The object also emits
// the functor classes of _Init's lambdas (0x6C8DC0 to 0x6C9370) and an
// unreferenced copy of _Exit's body at 0x6C7370.
#include "render/scene/RndDrawNodeCom.h"

#include <cmath>
#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/EntityHeaderCom.h"
#include "entity/core/InstanceCom.h"
#include "entity/core/TransCom.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "math/geometry/Plane.h"
#include "os/memory/MemMgr.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/lighting/lights/RndLightEnvironCom.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneInstanceCom.h"

// The class symbols of the instance pool component, "InstancePool"
// (vtable 0x18E7058). Names not in the reference map.
extern Symbol gInstancePoolComId;     // 0x19E35A0
extern Symbol gInstancePoolComClass;  // 0x19E35A8

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x6C93AC; see RndTypesetter.cpp). This object
// reads the -1 as the null object id. Names not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};          // 0x1AB05C8
[[maybe_unused]] int gDrawNodeGroupSize2D = 8;  // 0x1AB05CC
[[maybe_unused]] int gDrawNodeGroupSize3D = 4;  // 0x1AB05D0

bool IsNull(GameObjectId id) {
    return id.mId == gNullObjectId.mId;
}

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

// The object's instance pool component, found by its class id as
// GameObject::GetCom does. Name not in the reference map.
Component* FindInstancePoolCom(const GameObject* object) {
    for (const GameObject::ComIndex& index : object->mComs) {
        if (index.mId == gInstancePoolComId) {
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

// Whether the instanced entity changed since the instance component last
// polled. Name not in the reference map.
bool InstancedEntityChanged(const Component* instance) {
    return static_cast<const InstanceCom*>(instance)->mRuntime.mNeedsInstance;
}

// The resource an instance pool instances, at its offset 304 (the pool
// adds a reference when it sets it, 0x125426). Name not in the reference
// map.
EntityResource* GetPoolResource(const Component* pool) {
    return *reinterpret_cast<EntityResource* const*>(
        reinterpret_cast<const char*>(pool) + 304);
}

// A link of the instance pool's list of pooled objects, at the pool's
// offset 384; each link sits 56 bytes after the pointer to its object.
// Name not in the reference map.
struct PoolLink {
    PoolLink* mNext;

    GameObject* Object() const {
        return *reinterpret_cast<GameObject* const*>(
            reinterpret_cast<const char*>(this) - 56);
    }
};

PoolLink* GetPoolLinks(Component* pool) {
    return reinterpret_cast<PoolLink*>(
        reinterpret_cast<char*>(pool) + 384);
}

// The TransCom's "parent_dynamic", which keeps _CacheNodePointers from
// fixing the draw-node parent. Name not in the reference map.
bool IsTransParentDynamic(const TransCom* trans) {
    return trans->mParentDynamic;
}

// The draw node of the entity's root object, or null.
RndDrawNodeCom* GetRootDrawNode(const Entity* entity) {
    return entity->GetRoot()->GetCom<RndDrawNodeCom>();
}

// The show/hide bits "show_hide_flags" does not list: hidden (edited as
// "showing"), hidden from selection, show nothing, and the two selection
// bits. Name not in the reference map.
constexpr unsigned int kUnlistedShowHideFlags = 0xCC1;

// The allowed values of a show/hide flags property: each bit from 1 to
// 0x800 not in `excluded`, with its label and help, built under the
// temporary heap. Inlined into _Init three times. Name not in the
// reference map.
eastl::vector<AllowedValue<unsigned int>> ShowHideFlagValues(unsigned int excluded) {
    eastl::vector<AllowedValue<unsigned int>> values;
    unsigned int temp;
    MemPushTemp(temp, true, true);
    unsigned int flag = 1;
    do {
        if ((flag & excluded) == 0) {
            values.push_back(AllowedValue<unsigned int>{
                flag,
                String(GetShowHideFlagName(static_cast<int>(flag))),
                String(GetShowHideFlagHelp(static_cast<int>(flag)))});
        }
        flag <<= 1;
    } while ((flag & 0xFFE) != 0);
    MemPopTemp(temp);
    return values;
}

// Registers the float members "x", "y" and "z" of a Vector3 struct
// property. Name not in the reference map.
void RegisterVector3Members(PropRegistry& registry, const Vector3& value) {
    TypeSpecificMetadata(registry.RegisterProp("x", offsetof(Vector3, x), kPropertyFloat, 4), value.x)
        .mDefault = value.x;
    TypeSpecificMetadata(registry.RegisterProp("y", offsetof(Vector3, y), kPropertyFloat, 4), value.y)
        .mDefault = value.y;
    TypeSpecificMetadata(registry.RegisterProp("z", offsetof(Vector3, z), kPropertyFloat, 4), value.z)
        .mDefault = value.z;
}

// Registers a Sphere struct property's "center", a Vector3 struct with
// zero defaults, and "radius". Inlined into _Init for "local_sphere" and
// "world_sphere". Name not in the reference map.
void RegisterSphereMembers(PropRegistry& registry, const Sphere& value) {
    GroupMetadata& center = static_cast<GroupMetadata&>(
        *registry.RegisterProp("center", offsetof(Sphere, center), kPropertyStruct, sizeof(Vector3))
             .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const Vector3 centerType = {0.0F, 0.0F, 0.0F};
    // The struct registrations set this word; its reader is not identified.
    center.mKind = 1;
    RegisterVector3Members(center.mRegistry, centerType);
    TypeSpecificMetadata(registry.RegisterProp("radius", offsetof(Sphere, radius), kPropertyFloat, 4), value.radius)
        .mDefault = value.radius;
}

// Registers the float members "a" to "d" of a Plane struct property.
// Inlined into _Init for the world clip planes. Name not in the reference
// map.
void RegisterPlaneMembers(PropRegistry& registry, const Plane& value) {
    TypeSpecificMetadata(registry.RegisterProp("a", offsetof(Plane, a), kPropertyFloat, 4), value.a)
        .mDefault = value.a;
    TypeSpecificMetadata(registry.RegisterProp("b", offsetof(Plane, b), kPropertyFloat, 4), value.b)
        .mDefault = value.b;
    TypeSpecificMetadata(registry.RegisterProp("c", offsetof(Plane, c), kPropertyFloat, 4), value.c)
        .mDefault = value.c;
    TypeSpecificMetadata(registry.RegisterProp("d", offsetof(Plane, d), kPropertyFloat, 4), value.d)
        .mDefault = value.d;
}

// The property callbacks _Init installs (0x6C8CA0 to 0x6C8DB0). Names not
// in the reference map.
bool GetShowing(const PropAccessorArgs& args);
void SetShowing(const PropAccessorArgs& args);
void OnNestedLightEnvironmentChanged(const PropAccessorArgs& args);
void RemoveNestedLightEnvironment(const PropAccessorArgs& args);
void OnSphereRadiusMultChanged(const PropAccessorArgs& args);
bool GetWorldShowing(const PropAccessorArgs& args);

}  // namespace

// The object's statics, in the order of its static initializer (0x6C9380).
Symbol RndDrawNodeCom::sId("DrawNode");
Symbol RndDrawNodeCom::sClassName("DrawNode");
PropRegistry RndDrawNodeCom::sPropRegistry;
ComMetaData RndDrawNodeCom::sMetaData;

// Reconstructed from eboot.elf at 0x6C3D40. The runtime data's constructor
// is inlined.
RndDrawNodeCom::RndDrawNodeCom()
    : mLocalSphere(Sphere::sZero),
      mSphereRadiusMult(1.0F),
      mShowHideFlags(0),
      mInheritFrom(kInheritTransformParent),
      mEnvironment(gNullObjectId),
      mClipPlanes{gNullObjectId, gNullObjectId} {}

// Reconstructed from eboot.elf at 0x6C3E60.
RndDrawNodeCom::RuntimeData::RuntimeData()
    : mEntityRootSphereDirty(false),
      mLocalSphereDirty(true),
      mWorldSphereChanged(false),
      mWorldSphere(Sphere::sZero),
      mWorldShowHideFlags(0),
      mEnvironIndex(RndLightEnvironCom::kNoEnvironIndex),
      mNestedLightEnvironsDirty(false),
      mUseDefaultEnviron(false),
      mWorldClipPlanes{},
      mUseInstancedSphere(true),
      mInheritFromParentEntity(true),
      mEditorShowHideFlags(0),
      mReserved(false),
      mNeedsEntityRootSphere(false),
      mParentDynamic(false),
      mCounterClockwise(true),
      mInvWorldXfm(Transform::sID),
      mTransCom(nullptr),
      mInstance(nullptr),
      mInstancePool(nullptr),
      mEntityRoot(nullptr),
      mParent(nullptr),
      mEnvironParent(nullptr),
      mInstancedRoot(nullptr),
      mParentInstancePool(nullptr) {}

// Reconstructed from eboot.elf at 0x6C3EF0. The property array frees its
// storage.
RndDrawNodeCom::~RndDrawNodeCom() {}

// Reconstructed from eboot.elf at 0x6C3FE0.
void RndDrawNodeCom::SetLocalSphereFromWorld(const Sphere& worldSphere) {
    SetLocalSphere(MultiplyMinScale(worldSphere, mRuntime.mInvWorldXfm));
}

// Reconstructed from eboot.elf at 0x6C4060. Revision 1 data takes the
// scene's default environment, and the upgrades to revisions 2 and 3 move
// the show/hide bits up as flags were inserted. "showing" and
// "world_showing" are views of bit 0; the runtime properties are read-only
// debug properties that are not saved. "environment" shows only in a
// scene, and "nested_light_environments" only for an instancing object in
// a scene; it is read-only unless the instanced entities hold lights.
void RndDrawNodeCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription =
        "Holds hierarchical drawing-related info (bounding volume, environment, show/hide flags)";
    metadata.mAuthor = "Daniel Sproul";
    metadata.mCategory = ComMetaData::kCategoryRendering;
    metadata.mAllowedResources.push_back(TransEntityResource::Id());
    metadata.mLightweight = true;
    metadata.mAliases.push_back(Symbol("RndDrawNodeCom"));
    RndDrawNodeCom prototype;
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            RndDrawNodeCom* const node = static_cast<RndDrawNodeCom*>(args.mComponent);
            const int rev = args.mSavedRev;
            if (rev < 1) {
                node->mRuntime.mUseDefaultEnviron = true;
            }
            if (rev < 2) {
                node->mShowHideFlags = (node->mShowHideFlags & ~0x38U) | ((node->mShowHideFlags << 1) & 0x70);
            }
            if (rev < 3) {
                node->mShowHideFlags = (node->mShowHideFlags & ~0x70U) | ((node->mShowHideFlags & 0x70) << 1);
                node->mShowHideFlags = (node->mShowHideFlags & ~0xFEU) | ((node->mShowHideFlags << 2) & 0x3F8);
            }
        };
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry == &own) {
        own.SetCurrentRev(4);
        own.mRevisionFunc = onRevision;
    } else {
        registry.AddInheritedRegistry(sClassName, own);
    }

    const bool showingType = false;
    BoolMetadata& showing = TypeSpecificMetadata(registry.RegisterProp("showing", -1, kPropertyBool, 0), showingType);
    static_cast<void>(Symbol("prop"));
    showing.mGetter = GetShowing;
    showing.mSetter = SetShowing;
    showing.mSaves = false;
    showing.mDefault = true;

    IntMetadata<unsigned int>& flags = TypeSpecificMetadata(
        registry.RegisterProp("show_hide_flags", offsetof(RndDrawNodeCom, mShowHideFlags), kPropertyUInt, 4),
        prototype.mShowHideFlags);
    flags.mDefault = prototype.mShowHideFlags;
    static_cast<void>(Symbol("prop"));
    flags.mBitfield = true;
    flags.mAllowedValues = ShowHideFlagValues(kUnlistedShowHideFlags);

    IntMetadata<int>& inheritFrom = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "inherit_from", offsetof(RndDrawNodeCom, mInheritFrom), kPropertyInt, 4).mMetadata.Get());
    inheritFrom.mDefault = prototype.mInheritFrom;
    static_cast<void>(Symbol("prop"));
    inheritFrom.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kInheritTransformParent,
            String("Transform Parent"),
            String("Will inherit show/hide state and clip planes from my immediate Transform parent")},
        AllowedValue<int>{
            kInheritEntityRoot,
            String("Entity Root"),
            String("Will inherit show/hide state and clip planes from my entity root")}};
    inheritFrom.mAnimatable = false;

    GameObjectIdMetadata& environment = TypeSpecificMetadata(
        registry.RegisterProp("environment", offsetof(RndDrawNodeCom, mEnvironment), kPropertyGameObjectId, 4),
        prototype.mEnvironment);
    environment.mDefault = prototype.mEnvironment;
    static_cast<void>(Symbol("prop"));
    environment.mHelp = String("Which environment does this object receive light from?");
    environment.mRequiredComs = {RndLightEnvironCom::sId};
    environment.mIsHidden._Set([](const TagArgs& args) {
        return args.mComponent.mObject->mEntity->GetRoot()->GetCom<RndSceneCom>() == nullptr;
    });

    PropInfo& nested = registry.RegisterPropArray(
        "nested_light_environments",
        offsetof(RndDrawNodeCom, mNestedLightEnvironments),
        sizeof(PropArray<GameObjectId>));
    ArrayMetadata& nestedMetadata = static_cast<ArrayMetadata&>(*nested.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    nestedMetadata.mHelp = String("Which environments do lights contained in this instance contribute light to?");
    PropRegistry::RegisterPropArrayItem(nested, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId environType = {static_cast<unsigned int>(-1)};
    GameObjectIdMetadata& nestedEnviron = TypeSpecificMetadata(nestedMetadata.mItemInfo, environType);
    static_cast<void>(Symbol("item"));
    nestedEnviron.mRequiredComs = {RndLightEnvironCom::sId};
    nestedEnviron.mAllowsDuplicates = false;
    nestedEnviron.mAllowsNull = false;
    nestedEnviron.mOnChanged = OnNestedLightEnvironmentChanged;
    // The hook that replaces the default removal of an element
    // (_RemoveProp, 0x199300).
    nestedMetadata.mHook2 = reinterpret_cast<void*>(RemoveNestedLightEnvironment);
    nestedMetadata.mIsHidden._Set([](const TagArgs& args) {
        const GameObject* const object = args.mComponent.mObject;
        if (object->mEntity->GetRoot()->GetCom<RndSceneCom>() == nullptr) {
            return true;
        }
        return FindInstanceCom(object) == nullptr;
    });
    nestedMetadata.mReadOnly._Set([](const TagArgs& args) {
        const RndDrawNodeCom& node = static_cast<const RndDrawNodeCom&>(args.mComponent);
        Component* const instance = FindInstanceCom(node.mObject);
        if (instance == nullptr) {
            return true;
        }
        return !node._ContainsNestedLightsRecur(*instance);
    });

    GroupMetadata& localSphere = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "local_sphere", offsetof(RndDrawNodeCom, mLocalSphere), kPropertyStruct, sizeof(Sphere))
        .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const Sphere localSphereType = {};
    localSphere.mReadOnly = true;
    localSphere.mHelp = String("Local bounding sphere");
    RegisterSphereMembers(localSphere.mRegistry, localSphereType);

    FloatMetadata& radiusMult = TypeSpecificMetadata(
        registry.RegisterProp(
            "sphere_radius_mult", offsetof(RndDrawNodeCom, mSphereRadiusMult), kPropertyFloat, 4),
        prototype.mSphereRadiusMult);
    radiusMult.mDefault = prototype.mSphereRadiusMult;
    static_cast<void>(Symbol("prop"));
    radiusMult.mMin = 0.01F;
    radiusMult.mOnChanged = OnSphereRadiusMultChanged;

    IntMetadata<unsigned int>& editorFlags = TypeSpecificMetadata(
        registry.RegisterProp(
            "editor_show_hide_flags",
            offsetof(RndDrawNodeCom, mRuntime) + offsetof(RuntimeData, mEditorShowHideFlags),
            kPropertyUInt,
            4),
        prototype.mRuntime.mEditorShowHideFlags);
    editorFlags.mDefault = prototype.mRuntime.mEditorShowHideFlags;
    static_cast<void>(Symbol("prop"));
    editorFlags.mReadOnly = true;
    editorFlags.mDebug = true;
    editorFlags.mSaves = false;
    editorFlags.mBitfield = true;
    editorFlags.mAllowedValues = ShowHideFlagValues(0);

    const bool worldShowingType = false;
    BoolMetadata& worldShowing =
        TypeSpecificMetadata(registry.RegisterProp("world_showing", -1, kPropertyBool, 0), worldShowingType);
    static_cast<void>(Symbol("prop"));
    worldShowing.mReadOnly = true;
    worldShowing.mDebug = true;
    worldShowing.mGetter = GetWorldShowing;

    IntMetadata<unsigned int>& worldFlags = TypeSpecificMetadata(
        registry.RegisterProp(
            "world_show_hide_flags",
            offsetof(RndDrawNodeCom, mRuntime) + offsetof(RuntimeData, mWorldShowHideFlags),
            kPropertyUInt,
            4),
        prototype.mRuntime.mWorldShowHideFlags);
    worldFlags.mDefault = prototype.mRuntime.mWorldShowHideFlags;
    static_cast<void>(Symbol("prop"));
    worldFlags.mReadOnly = true;
    worldFlags.mDebug = true;
    worldFlags.mSaves = false;
    worldFlags.mBitfield = true;
    worldFlags.mAllowedValues = ShowHideFlagValues(0);

    IntMetadata<int>& environIndex = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "environ_index",
        offsetof(RndDrawNodeCom, mRuntime) + offsetof(RuntimeData, mEnvironIndex),
        kPropertyInt,
        4).mMetadata.Get());
    environIndex.mDefault = prototype.mRuntime.mEnvironIndex;
    static_cast<void>(Symbol("prop"));
    environIndex.mReadOnly = true;
    environIndex.mDebug = true;
    environIndex.mSaves = false;

    GroupMetadata& worldSphere = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "world_sphere",
        offsetof(RndDrawNodeCom, mRuntime) + offsetof(RuntimeData, mWorldSphere),
        kPropertyStruct,
        sizeof(Sphere)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const Sphere worldSphereType = {};
    worldSphere.mHelp = String("World bounding sphere (dynamically updated at runtime)");
    worldSphere.mReadOnly = true;
    worldSphere.mDebug = true;
    worldSphere.mSaves = false;
    RegisterSphereMembers(worldSphere.mRegistry, worldSphereType);

    GameObjectIdMetadata& clipPlane0 = TypeSpecificMetadata(
        registry.RegisterProp("clip_plane_0", offsetof(RndDrawNodeCom, mClipPlanes), kPropertyGameObjectId, 4),
        prototype.mClipPlanes[0]);
    clipPlane0.mDefault = prototype.mClipPlanes[0];
    static_cast<void>(Symbol("prop"));
    clipPlane0.mHelp =
        String("If specified, this object and its children will clip based on the target transform's Z-axis");
    GameObjectIdMetadata& clipPlane1 = TypeSpecificMetadata(
        registry.RegisterProp(
            "clip_plane_1", offsetof(RndDrawNodeCom, mClipPlanes) + sizeof(GameObjectId), kPropertyGameObjectId, 4),
        prototype.mClipPlanes[1]);
    clipPlane1.mDefault = prototype.mClipPlanes[1];
    static_cast<void>(Symbol("prop"));
    clipPlane1.mHelp =
        String("If specified, this object and its children will clip based on the target transform's Z-axis");

    GroupMetadata& worldClipPlane0 = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "world_clip_plane_0",
        offsetof(RndDrawNodeCom, mRuntime) + offsetof(RuntimeData, mWorldClipPlanes),
        kPropertyStruct,
        sizeof(Plane)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const Plane plane0Type;
    worldClipPlane0.mKind = 1;
    worldClipPlane0.mDebug = true;
    worldClipPlane0.mReadOnly = true;
    worldClipPlane0.mSaves = false;
    RegisterPlaneMembers(worldClipPlane0.mRegistry, plane0Type);

    GroupMetadata& worldClipPlane1 = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "world_clip_plane_1",
        offsetof(RndDrawNodeCom, mRuntime) + offsetof(RuntimeData, mWorldClipPlanes) + sizeof(Vector4),
        kPropertyStruct,
        sizeof(Plane)).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    const Plane plane1Type;
    worldClipPlane1.mKind = 1;
    worldClipPlane1.mDebug = true;
    worldClipPlane1.mReadOnly = true;
    worldClipPlane1.mSaves = false;
    RegisterPlaneMembers(worldClipPlane1.mRegistry, plane1Type);
}

// Reconstructed from eboot.elf at 0x6C5C80.
void RndDrawNodeCom::_GetPollDeps(
    eastl::vector<GameObjectId>& before,
    eastl::vector<GameObjectId>& after) {
    static_cast<void>(after);
    if (!IsNull(mEnvironment)) {
        before.push_back(mEnvironment);
    }
    before.push_back(mClipPlanes[0]);
    before.push_back(mClipPlanes[1]);
}

// Reconstructed from eboot.elf at 0x6C5EE0.
void RndDrawNodeCom::_GetComponentOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    follows.push_back(TransCom::sClassName);
    precedes.push_back(InstanceCom::sClassName);
    precedes.push_back(gInstancePoolComClass);
    precedes.push_back(RndSceneInstanceCom::sClassName);
}

// Reconstructed from eboot.elf at 0x6C61A0. Unlike the default, the class
// does not follow TransCom in the poll order.
void RndDrawNodeCom::_GetPollOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(InstanceCom::sClassName);
    follows.push_back(gInstancePoolComClass);
    follows.push_back(RndSceneInstanceCom::sClassName);
}

// Reconstructed from eboot.elf at 0x6C63B0. Data upgraded from revision 0
// takes the scene's default environment first.
bool RndDrawNodeCom::_OnResourcesLoaded() {
    if (mRuntime.mUseDefaultEnviron) {
        mRuntime.mUseDefaultEnviron = false;
        RndSceneCom* scene =
            mObject->mEntity->GetRoot()->GetCom<RndSceneCom>();
        if (scene != nullptr) {
            RndLightMgrCom* lightMgr = scene->GetLightMgr();
            if (lightMgr != nullptr) {
                RndLightEnvironCom* environ = lightMgr->GetDefaultEnviron();
                if (environ != nullptr) {
                    mEnvironment = environ->mObject->mId;
                }
            }
        }
    }
    _CacheNodePointers();
    return true;
}

// Inlined into _OnResourcesLoaded.
void RndDrawNodeCom::_CacheNodePointers() {
    GameObject* object = mObject;
    Entity* entity = object->mEntity;
    mRuntime.mTransCom = object->GetCom<TransCom>();
    mRuntime.mInstance = FindInstanceCom(object);
    mRuntime.mInstancePool = FindInstancePoolCom(object);
    mRuntime.mEntityRoot = GetRootDrawNode(entity);
    mRuntime.mParent = _FindParent();
    mRuntime.mEnvironParent = _FindEnvironParent();
    mRuntime.mInstancedRoot = _FindInstancedRoot();
    GameObject* parentObject = entity->mParentObject;
    mRuntime.mParentInstancePool =
        parentObject != nullptr ? FindInstancePoolCom(parentObject) : nullptr;

    // A transform up to the parent draw node that can change parents makes
    // the poll look the parent up again.
    mRuntime.mParentDynamic = false;
    if (mInheritFrom != kInheritTransformParent ||
        mRuntime.mEntityRoot == this) {
        return;
    }
    const GameObject* current = object;
    while (true) {
        const TransCom* trans = current->GetCom<TransCom>();
        if (IsTransParentDynamic(trans)) {
            mRuntime.mParentDynamic = true;
            return;
        }
        if (IsNull(trans->mTransParent)) {
            return;
        }
        const GameObject* parent = entity->GetObject(trans->mTransParent);
        if (parent == nullptr ||
            parent->GetCom<RndDrawNodeCom>() != nullptr) {
            return;
        }
        current = parent;
    }
}

// Reconstructed from eboot.elf at 0x6C68C0. The transform parents are
// assumed to have TransComs.
RndDrawNodeCom* RndDrawNodeCom::_FindParent() const {
    const Entity* entity = mObject->mEntity;
    RndDrawNodeCom* root = mRuntime.mEntityRoot;
    if (root == this) {
        if (!mRuntime.mInheritFromParentEntity) {
            return nullptr;
        }
        const GameObject* parentObject = entity->mParentObject;
        if (parentObject == nullptr) {
            return nullptr;
        }
        return parentObject->GetCom<RndDrawNodeCom>();
    }
    if (mInheritFrom != kInheritTransformParent || mObject == nullptr) {
        return root;
    }
    const GameObject* current = mObject;
    while (true) {
        const TransCom* trans = current->GetExistingCom<TransCom>();
        if (IsNull(trans->mTransParent)) {
            return root;
        }
        const GameObject* parent = entity->GetObject(trans->mTransParent);
        if (parent == nullptr) {
            return root;
        }
        RndDrawNodeCom* node = parent->GetCom<RndDrawNodeCom>();
        if (node != nullptr) {
            return node;
        }
        current = parent;
    }
}

// Reconstructed from eboot.elf at 0x6C6A30.
RndDrawNodeCom* RndDrawNodeCom::_FindEnvironParent() const {
    if (mRuntime.mEntityRoot != this) {
        return mRuntime.mEntityRoot;
    }
    return _FindParent();
}

// Reconstructed from eboot.elf at 0x6C6A50.
RndDrawNodeCom* RndDrawNodeCom::_FindInstancedRoot() const {
    if (mRuntime.mInstance != nullptr) {
        const Entity* entity = GetInstancedEntity(mRuntime.mInstance);
        return entity != nullptr ? GetRootDrawNode(entity) : nullptr;
    }
    if (mRuntime.mInstancePool == nullptr) {
        return nullptr;
    }
    EntityResource* resource = GetPoolResource(mRuntime.mInstancePool);
    if (resource == nullptr || resource->Fail()) {
        return nullptr;
    }
    const Entity* entity = resource->mEntity;
    return entity != nullptr ? GetRootDrawNode(entity) : nullptr;
}

// Inlined into _Enter, _PollDrawNode and _EditEnter.
void RndDrawNodeCom::_UpdateInvWorldXfm() {
    const Transform& world = mRuntime.mTransCom->mWorldXfm;
    Transform& inverse = mRuntime.mInvWorldXfm;
    float det;
    Invert(world.m, inverse.m, &det);
    const Vector3 v = world.v;
    const Hmx::Matrix3& m = inverse.m;
    inverse.v.x = -(v.x * m.x.x + v.y * m.y.x + v.z * m.z.x);
    inverse.v.y = -(v.x * m.x.y + v.y * m.y.y + v.z * m.z.y);
    inverse.v.z = -(v.x * m.x.z + v.y * m.y.z + v.z * m.z.z);
    mRuntime.mCounterClockwise = !(det < 0.0F);
}

// Inlined into _Enter, _PollDrawNode and _EditEnter. TransCom's dirty bit
// 2 marks a world transform that changed this frame.
void RndDrawNodeCom::_SyncWorldState() {
    RndDrawNodeCom* const root = mRuntime.mEntityRoot;
    if (mRuntime.mEntityRootSphereDirty) {
        if (mRuntime.mNeedsEntityRootSphere) {
            if (mRuntime.mParentInstancePool != nullptr) {
                _ComputeInstancePoolLocalSphere();
            } else {
                _ComputeEntityRootLocalSphere();
            }
        }
        mRuntime.mEntityRootSphereDirty = false;
    }
    if (mRuntime.mInstancedRoot != nullptr && mRuntime.mUseInstancedSphere) {
        SetLocalSphere(MultiplyMinScale(
            mRuntime.mInstancedRoot->mRuntime.mWorldSphere,
            mRuntime.mInvWorldXfm));
    }

    const TransCom* trans = mRuntime.mTransCom;
    if (mRuntime.mLocalSphereDirty || (trans->mDirtyFlags & 2) != 0) {
        Multiply(mLocalSphere, trans->mWorldXfm, mRuntime.mWorldSphere);
        mRuntime.mWorldSphere.radius *= mSphereRadiusMult;
        mRuntime.mLocalSphereDirty = false;
        mRuntime.mWorldSphereChanged = true;
        if (root != this && mRuntime.mEntityRoot != nullptr) {
            mRuntime.mEntityRoot->mRuntime.mEntityRootSphereDirty = true;
        }
    } else {
        mRuntime.mWorldSphereChanged = false;
    }

    // A clip plane passes through its object and faces its z axis; without
    // an object it is the parent's, or zero.
    const Entity* entity = mObject->mEntity;
    mRuntime.mWorldShowHideFlags = mShowHideFlags;
    for (int i = 0; i < 2; ++i) {
        const GameObject* planeObject = IsNull(mClipPlanes[i])
                                            ? nullptr
                                            : entity->GetObject(mClipPlanes[i]);
        Vector4& plane = mRuntime.mWorldClipPlanes[i];
        if (planeObject == nullptr) {
            if (mRuntime.mParent != nullptr) {
                plane = mRuntime.mParent->mRuntime.mWorldClipPlanes[i];
            } else {
                plane = Vector4{0.0F, 0.0F, 0.0F, 0.0F};
            }
            continue;
        }
        const Transform& xfm = planeObject->GetCom<TransCom>()->mWorldXfm;
        const Vector3& axis = xfm.m.z;
        const float length =
            std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
        const float scale = length != 0.0F ? 1.0F / length : 0.0F;
        plane.x = scale * axis.x;
        plane.y = scale * axis.y;
        plane.z = scale * axis.z;
        plane.w =
            -(plane.x * xfm.v.x + plane.y * xfm.v.y + plane.z * xfm.v.z);
    }
    if (mRuntime.mParent != nullptr) {
        mRuntime.mWorldShowHideFlags |=
            mRuntime.mParent->mRuntime.mWorldShowHideFlags &
            kParentInheritedFlags;
    }
    const RndDrawNodeCom* environParent = mRuntime.mEnvironParent;
    if (environParent != nullptr) {
        mRuntime.mWorldShowHideFlags |=
            environParent->mRuntime.mWorldShowHideFlags &
            kEnvironInheritedFlags;
    }

    // In a scene the environment object gives the index; elsewhere it comes
    // from the environment's draw node.
    if (!_IsInScene()) {
        mRuntime.mEnvironIndex = environParent != nullptr
                                     ? environParent->mRuntime.mEnvironIndex
                                     : 0;
        return;
    }
    mRuntime.mEnvironIndex = RndLightEnvironCom::kNoEnvironIndex;
    if (IsNull(mEnvironment)) {
        return;
    }
    const GameObject* environObject = entity->GetObject(mEnvironment);
    if (environObject == nullptr) {
        return;
    }
    const RndLightEnvironCom* environ =
        environObject->GetCom<RndLightEnvironCom>();
    if (environ != nullptr) {
        mRuntime.mEnvironIndex = environ->mRuntime.mEnvironIndex;
    }
}

// Reconstructed from eboot.elf at 0x6C6B50. An entity that polls only while
// on screen, at any level above the object, needs its root's sphere.
void RndDrawNodeCom::_Enter() {
    mRuntime.mParent = _FindParent();
    _UpdateInvWorldXfm();
    _SyncWorldState();
    mRuntime.mNeedsEntityRootSphere = false;
    for (const Entity* entity = mObject->mEntity; entity != nullptr;
         entity = entity->GetParent()) {
        const EntityHeaderCom* header =
            entity->GetRoot()->GetCom<EntityHeaderCom>();
        if (header != nullptr && !header->mDrivesParent &&
            header->mInstancePollingMode == 2) {
            mRuntime.mNeedsEntityRootSphere = true;
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x6C7350. An exiting instance or object
// makes the entity root rebuild its sphere.
void RndDrawNodeCom::_Exit(DestroyType type) {
    if (static_cast<unsigned int>(type) < kDestroyInstance) {
        return;
    }
    RndDrawNodeCom* root = mRuntime.mEntityRoot;
    if (root != nullptr && root != this) {
        root->mRuntime.mEntityRootSphereDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x6C7390.
void RndDrawNodeCom::_Poll() {
    _PollDrawNode();
}

// Reconstructed from eboot.elf at 0x6C73A0. The entity root looks its
// parents up again after its entity changed parents (Entity flag 0x4000).
void RndDrawNodeCom::_PollDrawNode() {
    const Entity* entity = mObject->mEntity;
    if (mRuntime.mEntityRoot == this && (entity->mFlags & 0x4000) != 0) {
        mRuntime.mParent = _FindParent();
        mRuntime.mEnvironParent = _FindParent();
    } else if (mRuntime.mParentDynamic) {
        mRuntime.mParent = _FindParent();
    }
    const Component* instance = mRuntime.mInstance;
    if (instance != nullptr && InstancedEntityChanged(instance)) {
        const Entity* instanced = GetInstancedEntity(instance);
        mRuntime.mInstancedRoot =
            instanced != nullptr ? GetRootDrawNode(instanced) : nullptr;
    }
    if ((mRuntime.mTransCom->mDirtyFlags & 2) != 0) {
        _UpdateInvWorldXfm();
    }
    _SyncWorldState();
}

// Reconstructed from eboot.elf at 0x6C7B60. In the edit mode every entity
// root keeps its sphere.
void RndDrawNodeCom::_EditEnter() {
    mRuntime.mNeedsEntityRootSphere = true;
    _UpdateInvWorldXfm();
    _SyncWorldState();
}

// Reconstructed from eboot.elf at 0x6C8250. _SyncNestedLightEnvirons is
// inlined.
void RndDrawNodeCom::_EditPoll() {
    if (mRuntime.mNestedLightEnvironsDirty) {
        _SyncNestedLightEnvirons();
    }
    _PollDrawNode();
}

// Reconstructed from eboot.elf at 0x6C82F0.
void RndDrawNodeCom::_SyncNestedLightEnvirons() {
    mRuntime.mNestedLightEnvironsDirty = false;
    Component* instance = FindInstanceCom(mObject);
    if (instance == nullptr) {
        mNestedLightEnvironments.Resize(0);
        return;
    }
    RndLightCom::RemoveNestedLightsFromBookkeeping(instance);
    RndLightCom::AddNestedLightsToBookkeeping(instance);
}

// Reconstructed from eboot.elf at 0x6C8380.
void RndDrawNodeCom::_EditExit(DestroyType type) {
    if (static_cast<unsigned int>(type) < kDestroyInstance) {
        return;
    }
    RndDrawNodeCom* root = mRuntime.mEntityRoot;
    if (root != nullptr && root != this) {
        root->mRuntime.mEntityRootSphereDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x6C83A0.
bool RndDrawNodeCom::_IsInScene() const {
    return mObject->mEntity->GetRoot()->GetCom<RndSceneCom>() != nullptr;
}

// Reconstructed from eboot.elf at 0x6C8400.
bool RndDrawNodeCom::_ContainsNestedLights() const {
    Component* instance = FindInstanceCom(mObject);
    return instance != nullptr && _ContainsNestedLightsRecur(*instance);
}

// Reconstructed from eboot.elf at 0x6C8470.
bool RndDrawNodeCom::_ContainsNestedLightsRecur(Component& instance) const {
    const Entity* entity = GetInstancedEntity(&instance);
    if (entity == nullptr) {
        return false;
    }
    for (const GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (object->GetBaseCom<RndLightCom>() != nullptr) {
            return true;
        }
        Component* nested = FindInstanceCom(object);
        if (nested != nullptr && _ContainsNestedLightsRecur(*nested)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6C8590.
int RndDrawNodeCom::_GetNumNestedLights() const {
    Component* instance = FindInstanceCom(mObject);
    return instance != nullptr ? _GetNumNestedLightsRecur(*instance) : 0;
}

// Reconstructed from eboot.elf at 0x6C8600.
int RndDrawNodeCom::_GetNumNestedLightsRecur(Component& instance) const {
    const Entity* entity = GetInstancedEntity(&instance);
    if (entity == nullptr) {
        return 0;
    }
    int count = 0;
    for (const GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (object->GetBaseCom<RndLightCom>() != nullptr) {
            ++count;
        }
        Component* nested = FindInstanceCom(object);
        if (nested != nullptr) {
            count += _GetNumNestedLightsRecur(*nested);
        }
    }
    return count;
}

// Reconstructed from eboot.elf at 0x6C8730. The sphere encloses the world
// spheres of the entity's other draw nodes, moved into the object's space.
void RndDrawNodeCom::_ComputeEntityRootLocalSphere() {
    Sphere sphere = Sphere::sZero;
    const Entity* entity = mObject->mEntity;
    for (const GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        if (object == mObject) {
            continue;
        }
        const RndDrawNodeCom* node = object->GetCom<RndDrawNodeCom>();
        if (node != nullptr) {
            sphere.GrowToContain(node->mRuntime.mWorldSphere);
        }
    }
    SetLocalSphere(MultiplyMinScale(sphere, mRuntime.mInvWorldXfm));
}

// Reconstructed from eboot.elf at 0x6C88A0. The binary assumes every
// pooled object has a draw node.
void RndDrawNodeCom::_ComputeInstancePoolLocalSphere() {
    Sphere sphere = Sphere::sZero;
    PoolLink* links = GetPoolLinks(mRuntime.mParentInstancePool);
    for (PoolLink* link = links->mNext; link != links; link = link->mNext) {
        const RndDrawNodeCom* node =
            link->Object()->GetCom<RndDrawNodeCom>();
        sphere.GrowToContain(node->mRuntime.mWorldSphere);
    }
    SetLocalSphere(MultiplyMinScale(sphere, mRuntime.mInvWorldXfm));
}

// Reconstructed from eboot.elf at 0x6C89F0.
Symbol RndDrawNodeCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x6C8A00.
Symbol RndDrawNodeCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x6C8A10.
int RndDrawNodeCom::CurrentRev() const {
    return const_cast<RndDrawNodeCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x6C8A30.
bool RndDrawNodeCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr;
         metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x6C8A60.
Component* RndDrawNodeCom::AsComponent() {
    return this;
}

// Inlined into _Imprint (0x6C8A70): the properties are copied, the
// property array through PropArrayBase::_Copy, and the runtime data starts
// fresh.
RndDrawNodeCom::RndDrawNodeCom(const RndDrawNodeCom& other)
    : Component(other),
      mLocalSphere(other.mLocalSphere),
      mSphereRadiusMult(other.mSphereRadiusMult),
      mShowHideFlags(other.mShowHideFlags),
      mInheritFrom(other.mInheritFrom),
      mEnvironment(other.mEnvironment),
      mClipPlanes{other.mClipPlanes[0], other.mClipPlanes[1]} {
    mNestedLightEnvironments._Copy(other.mNestedLightEnvironments);
}

// Reconstructed from eboot.elf at 0x6C8A70. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* RndDrawNodeCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<RndDrawNodeCom*>(
        (reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) RndDrawNodeCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x6C8C80.
PropRegistry& RndDrawNodeCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x6C8C90.
ComMetaData& RndDrawNodeCom::_GetMetaData() {
    return sMetaData;
}

namespace {

// Reconstructed from eboot.elf at 0x6C8CA0.
bool GetShowing(const PropAccessorArgs& args) {
    return (static_cast<RndDrawNodeCom*>(args.mComponent)->mShowHideFlags & RndDrawNodeCom::kHidden) == 0;
}

// Reconstructed from eboot.elf at 0x6C8CB0.
void SetShowing(const PropAccessorArgs& args) {
    RndDrawNodeCom* const node = static_cast<RndDrawNodeCom*>(args.mComponent);
    const unsigned int flags = node->mShowHideFlags;
    node->mShowHideFlags = *static_cast<const bool*>(args.mValue) ? flags & ~RndDrawNodeCom::kHidden
                                                                  : flags | RndDrawNodeCom::kHidden;
}

// Reconstructed from eboot.elf at 0x6C8CD0.
void OnNestedLightEnvironmentChanged(const PropAccessorArgs& args) {
    static_cast<RndDrawNodeCom*>(args.mComponent)->mRuntime.mNestedLightEnvironsDirty = true;
}

// Reconstructed from eboot.elf at 0x6C8CE0. Removes the element the path's
// last index names, as the default removal does, and marks the nested
// lights for re-registration.
void RemoveNestedLightEnvironment(const PropAccessorArgs& args) {
    RndDrawNodeCom* const node = static_cast<RndDrawNodeCom*>(args.mComponent);
    PropArrayBase& environs = node->mNestedLightEnvironments;
    const unsigned long index = (*static_cast<const PropPath*>(args.mPath))[-1].mValue;
    environs.Destruct(1, environs.ElementAt(index));
    const unsigned int last = environs.mSize - 1;
    if (static_cast<unsigned int>(index) < last) {
        environs.Move(last - index, environs.ElementAt(index), environs.ElementAt(index + 1));
    }
    environs.mSize = last;
    node->mRuntime.mNestedLightEnvironsDirty = true;
}

// Reconstructed from eboot.elf at 0x6C8DA0.
void OnSphereRadiusMultChanged(const PropAccessorArgs& args) {
    static_cast<RndDrawNodeCom*>(args.mComponent)->mRuntime.mLocalSphereDirty = true;
}

// Reconstructed from eboot.elf at 0x6C8DB0.
bool GetWorldShowing(const PropAccessorArgs& args) {
    return (static_cast<RndDrawNodeCom*>(args.mComponent)->mRuntime.mWorldShowHideFlags &
            RndDrawNodeCom::kHidden) == 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x404430. The binary emits the factory
// with the other render factories.
Component* RndDrawNodeCom::_Create() {
    return new RndDrawNodeCom();
}

// Reconstructed from eboot.elf at 0x3EE650. The binary emits the registration
// in render/RndInit.o.
void RndDrawNodeCom::Init() {
    InitComponentClass<RndDrawNodeCom>();
}
