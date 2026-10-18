#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "math/matrix/Matrix3.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

// The transform component (entity/TransCom.o; "Transform", 128 bytes,
// vtable 0x18E8AC0). The class registers through Init, emitted in
// entity/Utl.o, and _Init. The handler, the game-mode enter and the poll,
// which computes the world transform, are declared but not reconstructed.
class TransCom : public Component {
public:
    // The local transform as scale, Euler angles and position: the value
    // of a transform property (PropXfm), which the registry's "reset"
    // assigns and the class's constructor copies. The type and field names
    // are not in the reference map.
    struct LocalXfm {
        LocalXfm() = default;
        // Inlined into the constructor and the "reset" action. Name not in
        // the reference map.
        LocalXfm(const PropXfm& xfm) : mScale(xfm.mScale), mEuler(xfm.mRotation), mPos(xfm.mTranslation) {}

        // Splits the rotation into the row scales and the Euler angles
        // (through MakeEuler at 0x215B10). With `keepScaleSigns`, which
        // RndDefaults passes, the scale signs come from the stored scale.
        // Name not in the reference map.
        void SetRotation(const Hmx::Matrix3& rotation, bool keepScaleSigns);  // 0x127730

        Vector3 mScale;
        Vector3 mEuler;
        Vector3 mPos;
    };

    // Inlined into the factory (0x112EF60), Init and _Init. The local
    // transform starts as PropXfm::sIdentity and the dirty flags as 3.
    TransCom();
    // Inlined into _Imprint (0x1B4D30): the properties are copied and the
    // run-time state starts afresh.
    TransCom(const TransCom& other);
    // Slots 0-1: 0x1B3A80 and 0x1B4C90.
    ~TransCom() override;
    // Slot 2 at 0x1B47F0: "get_local_pos", "set_local_pos" and
    // "flag_world_changed". Not reconstructed.
    DataNode Handle(DataArray* msg, bool warn) override;
    Symbol GetId() const override;          // slot 4: 0x1B4CB0
    Symbol GetClassName() const override;   // slot 5: 0x1B4CC0
    int CurrentRev() const override;        // slot 7: 0x1B4CD0
    bool IsA(Symbol type) const override;   // slot 8: 0x1B4CF0
    Component* AsComponent() override;      // slot 9: 0x1B4D20
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x1B4D30
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x1B4EA0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x1B4EB0
    // Slot 27: the parent polls first.
    void _GetPollDeps(
        eastl::vector<GameObjectId>& before,
        eastl::vector<GameObjectId>& after) override;  // 0x1B46E0
    // Slot 29: clears a parent that names the object itself.
    bool _OnResourcesLoaded() override;  // 0x1B4010
    // Slot 31 at 0x1B4040: decides mUseEntityParent (from the instancing
    // object's InstanceCom::IsDrivenByChild) and polls. Not reconstructed.
    void _Enter() override;
    // Slot 33 at 0x1B4140: applies a parent change and recomputes the world
    // transform from the parent's by mInheritType. Not reconstructed.
    void _Poll() override;
    // Slot 35: marks the local transform dirty.
    void _OnActivate() override;  // 0x1B47C0
    void _EditEnter() override;   // slot 36: 0x1B47D0
    void _EditPoll() override;    // slot 38: 0x1B47E0

    // Sets the local transform and marks it dirty. Inlined into
    // RndDefaults::_SyncSpotlight (0x6BFD40) and
    // RndDefaults::_CreateBackupLighting (0x6BF4F0). Name not in the
    // reference map.
    void SetLocalXfm(const Transform& xfm) {
        mLocalXfm.SetRotation(xfm.m, true);
        mLocalXfm.mPos = xfm.v;
        mDirtyFlags |= 1;
    }

    // Makes the object the parent, or the invalid id for none. `keepWorld`
    // also marks the world transform for keeping (flag 0x10).
    void SetTransParent(GameObjectId parent, bool keepWorld);  // 0x1B3A90

    // The factory; emitted in entity/Utl.o.
    static Component* _Create();  // 0x112EF60
    // Registers the class (InitComponentClass); emitted in entity/Utl.o.
    static void Init();  // 0x112D830
    // Describes the class and registers its properties.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x1AFFB0

    // "Transform", constructed by the static initializer at 0x1BA9D0; the
    // class's virtual at slot 4 returns it.
    static Symbol sId;  // 0x19E46E8
    // Also "Transform"; the class symbol that GameObject::CreateComponent
    // takes, returned by the virtual at slot 5. Name not in the reference
    // map.
    static Symbol sClassName;           // 0x19E46F0
    static PropRegistry sPropRegistry;  // 0x19E4700
    static ComMetaData sMetaData;       // 0x19E47A0

    // Field names are not in the reference map. The property registration
    // (0x1AFFB0) builds a prototype whose fields give the defaults below.
    LocalXfm mLocalXfm;
    // The parent object, or the invalid id (-1). The map has
    // TransCom::SetTransParent(GameObjectId, bool); the poll (0x1B4140)
    // reads it.
    GameObjectId mTransParent;
    // "parent_dynamic": the parent may change from frame to frame (for
    // example when a state graph animates it). A bool property at 64
    // (registry 0x1AFFB0); the three bytes after it are padding.
    bool mParentDynamic;
    // Which parent attributes are inherited: 0 the entire transform, 1 scale
    // and rotation, 2 translation. Registered as "inherit_type".
    int mInheritType;
    // Flags: 1 marks the local transform dirty, 4 the parent changed and
    // 0x10 a parent change that keeps the world transform.
    unsigned int mDirtyFlags;
    // Makes the entity's parent object (Entity+304) the parent instead of
    // mTransParent. Name inferred from its use in the poll (0x1B4140).
    bool mUseEntityParent;
    unsigned char mPadding[3];  // Never read or written.
    Transform mWorldXfm;
};

static_assert(offsetof(TransCom, mLocalXfm) == 24);
static_assert(offsetof(TransCom::LocalXfm, mEuler) == offsetof(PropXfm, mRotation));
static_assert(offsetof(TransCom::LocalXfm, mPos) == offsetof(PropXfm, mTranslation));
static_assert(sizeof(TransCom::LocalXfm) == sizeof(PropXfm));
static_assert(offsetof(TransCom, mTransParent) == 60);
static_assert(offsetof(TransCom, mParentDynamic) == 64);
static_assert(offsetof(TransCom, mInheritType) == 68);
static_assert(offsetof(TransCom, mDirtyFlags) == 72);
static_assert(offsetof(TransCom, mUseEntityParent) == 76);
static_assert(offsetof(TransCom, mWorldXfm) == 80);
static_assert(sizeof(TransCom) == 128);

// The transform helpers of entity/TransUtl.o. Only the one the registry
// uses is declared.
class TransUtl {
public:
    // The local transform that gives the component the world transform,
    // relative to its parent. Not reconstructed.
    static TransCom::LocalXfm CalcNewLocalXfm(const TransCom& trans, const Transform& world);  // 0x1C8220
};

// The objects of the entity that may become the object's parent, which
// must hold a TransCom, after the invalid id when `allowNull` is set. Emitted in entity/Utl.o. The map's signature
// is GetValidTransParentsFromObject(ObjPtr const&,
// eastl::vector<GameObjectId>&, bool); this build passes the object. Not
// reconstructed.
void GetValidTransParentsFromObject(
    const GameObject* object,
    eastl::vector<GameObjectId>& parents,
    bool allowNull);  // 0x1141C0
