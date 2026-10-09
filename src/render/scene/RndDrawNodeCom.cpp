// render/RndDrawNodeCom.o (0x6C3D40 to 0x6C944F). The object also emits
// the registry's property callbacks (0x6C8CA0 to 0x6C8DBA) and the functor
// classes of _Init (0x6C8DC0 to 0x6C9370), which stay with _Init, and an
// unreferenced copy of _Exit's body at 0x6C7370.
#include "render/scene/RndDrawNodeCom.h"

#include <cmath>
#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/InstanceCom.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropRegistry.h"
#include "render/lighting/RndLightMgrCom.h"
#include "render/lighting/lights/RndLightCom.h"
#include "render/lighting/lights/RndLightEnvironCom.h"
#include "render/scene/RndSceneCom.h"
#include "render/scene/RndSceneInstanceCom.h"

// The class symbol of the instance component, "Instance" (sId 0x19E3278,
// vtable 0x18E6D58), which EntityResource.o also uses. Name not in the
// reference map.
extern Symbol gEntityInstanceComId;  // 0x19E3280
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
    if (gEntityInstanceComId == Symbol()) {
        return nullptr;
    }
    for (const GameObject::ComIndex& index : object->mComs) {
        if (index.mBaseId == gEntityInstanceComId) {
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

// The entity an instance component instances, at its offset 80. The class
// is not modelled. Name not in the reference map.
Entity* GetInstancedEntity(const Component* instance) {
    return *reinterpret_cast<Entity* const*>(
        reinterpret_cast<const char*>(instance) + 80);
}

// The instance component's byte at 122, which the poll takes as a sign
// that the instanced entity changed. The evidence for the meaning is weak.
// Name not in the reference map.
bool InstancedEntityChanged(const Component* instance) {
    return reinterpret_cast<const unsigned char*>(instance)[122] != 0;
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

// The TransCom byte at 64 (TransCom::mReserved's first byte), which keeps
// _CacheNodePointers from fixing the draw-node parent. Its writer is not
// identified, so the meaning is weak. Name not in the reference map.
bool IsTransParentDynamic(const TransCom* trans) {
    return *reinterpret_cast<const unsigned char*>(&trans->mReserved) != 0;
}

// The draw node of the entity's root object, or null.
RndDrawNodeCom* GetRootDrawNode(const Entity* entity) {
    return entity->GetRoot()->GetCom<RndDrawNodeCom>();
}

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
    precedes.push_back(gEntityInstanceComId);
    precedes.push_back(gInstancePoolComClass);
    precedes.push_back(RndSceneInstanceCom::sClassName);
}

// Reconstructed from eboot.elf at 0x6C61A0. Unlike the default, the class
// does not follow TransCom in the poll order.
void RndDrawNodeCom::_GetPollOrderDeps(
    eastl::vector<Symbol>& follows,
    eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(gEntityInstanceComId);
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
        const InstanceCom* instance =
            entity->GetRoot()->GetCom<InstanceCom>();
        if (instance != nullptr && !instance->mDrivesParent &&
            instance->mInstancePollingMode == 2) {
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

// Reconstructed from eboot.elf at 0x404430. The binary emits the factory
// with the other render factories.
Component* RndDrawNodeCom::_Create() {
    return new RndDrawNodeCom();
}
