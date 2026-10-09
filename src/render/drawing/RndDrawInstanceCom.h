#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "math/vector/Vector4.h"
#include "utl/containers/LinkedList.h"
#include "utl/containers/VectorAdapter.h"
#include "utl/text/Symbol.h"

class Entity;
class RndDrawNodeCom;
struct RndDrawInstance;
class RndDrawInstanceCom;
class RndMaterialCom;
class RndMaterialReferenceCom;
class RndSceneDrawer;
class TransCom;

// A scene level of detail. The draw-instance components keep one instance
// list per level; there are three when RndConfig::mUseLod is set and one
// otherwise. The map names the type; the enumerator names are not in the
// reference map.
enum RndSceneLod : unsigned int {
    kNumSceneLods = 3,
};

// The base of the components that hand drawable instances to the scene
// drawer (render/RndDrawInstanceCom.o, 0x6C1500-0x6C3D3F). Its class id is
// "DrawInstance". A subclass builds its instance lists, one per scene level,
// and passes them to InitDrawInstances; the component registers with the
// scene drawer when it enters and refreshes its instances every poll through
// slot 44. The vtable at 0x1937BB0 has 46 slots. The object is 192 bytes.
// Field names are not in the reference map; the properties are named after
// the registry (0x6C17F0).
class RndDrawInstanceCom : public Component {
public:
    // The members from 72 that are not properties; the map's
    // RndDrawInstanceCom::RuntimeData. Its constructor is inlined into the
    // component's constructor (0x6C1500) and _Imprint (0x6C3790).
    struct RuntimeData {
        RuntimeData()
            : mReserved(false),
              mRegistered(false),
              mInstancesInited(false),
              mRegistrationSuppressed(false),
              mSortKey(0),
              mActiveSortKey(0),
              mDrawInstances(),
              mTrans(nullptr),
              mDrawNode(nullptr),
              mMaterial(nullptr),
              mMaterialReference(nullptr),
              mDefaultMaterial(nullptr) {}

        // Cleared by the constructor; no reader is identified.
        bool mReserved;
        // Set once the component tried to register with its scene drawer.
        bool mRegistered;
        // Set by InitDrawInstances; the poll syncs the instances only then.
        bool mInstancesInited;
        // Keeps the component from registering. Its writer is not
        // identified, so the name is weak.
        bool mRegistrationSuppressed;
        // A sort key the instances take (RndDrawInstance::mSortKey) while
        // the global at 0x19B03C1 is set, and zero otherwise. Its writers
        // are not identified, so the names are weak.
        int mSortKey;
        int mActiveSortKey;
        // The instance list of each scene level.
        VectorAdapter<RndDrawInstance> mDrawInstances[kNumSceneLods];
        // The components found when the resources loaded
        // (_OnResourcesLoaded): the object's transform, draw node and
        // material, the material reference when there is no material, and
        // the class's default material.
        TransCom* mTrans;
        RndDrawNodeCom* mDrawNode;
        RndMaterialCom* mMaterial;
        RndMaterialReferenceCom* mMaterialReference;
        RndMaterialCom* mDefaultMaterial;
        // The link in the scene drawer's list of registered components.
        LinkedList::Node mSceneDrawerLink;
    };

    RndDrawInstanceCom();  // 0x6C1500
    // Copies the properties for an imprint; the run-time state starts
    // afresh. Inlined into the _Imprint of the class (0x6C3790) and of
    // RndMeshCom (0x5CA6A0). Not in the reference map.
    RndDrawInstanceCom(const RndDrawInstanceCom& other)
        : Component(other),
          mBillboarding(other.mBillboarding),
          mSortBy(other.mSortBy),
          mSortingHint(other.mSortingHint),
          mLods(other.mLods),
          mExtraData(other.mExtraData),
          mExtraData1(other.mExtraData1),
          mRuntime() {}
    // Slots 0-1: 0x6C1590, 0x6C15C0. The link leaves the drawer's list.
    ~RndDrawInstanceCom() override;

    Symbol GetId() const override;             // slot 4: 0x6C3710
    Symbol GetClassName() const override;      // slot 5: 0x6C3720
    int CurrentRev() const override;           // slot 7: 0x6C3730
    bool IsA(Symbol type) const override;      // slot 8: 0x6C3750
    Component* AsComponent() override;         // slot 9: 0x6C3780
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x6C3790
    // Slot 18 at 0x6C2D70: the component follows the draw node, the
    // material and the material reference.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;
    // Slot 20 at 0x6C2B30: a component destroyed alone collapses the draw
    // node's sphere onto the object's position.
    void _PreDestroy(DestroyType type) override;
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x6C3900
    ComMetaData& _GetMetaData() override;       // slot 23: 0x6C3910
    // Slot 29 at 0x6C2C00: caches the related components.
    bool _OnResourcesLoaded() override;
    // Slots 31-33 at 0x6C2F80, 0x6C3150 and 0x6C3260.
    void _Enter() override;
    void _Exit(DestroyType type) override;
    void _Poll() override;
    // Slots 34-35 at 0x6C32E0 and 0x6C3380: hide and show the instances.
    void _OnDeactivate() override;
    void _OnActivate() override;
    // Slots 36-38 at 0x6C3420, 0x6C3430 and 0x6C34C0.
    void _EditEnter() override;
    void _EditExit(DestroyType type) override;
    void _EditPoll() override;

    // Slot 41 at 0x6C34D0: the number of instances at the level, here one
    // when the level is in "lods". The map's signature starts with an
    // ObjPtr const&.
    virtual unsigned long _GetNumDrawInstancesImpl(RndSceneLod lod) const;
    // Slot 42 at 0x6C34E0: the material drawn when the object has none, the
    // renderer's unlit default. Name not in the reference map.
    virtual RndMaterialCom* _GetDefaultMaterial() const;
    // Slots 43-44 at 0x6C3920 and 0x6C3930: fill in the subclass's part of
    // a level's instances once, and every poll; empty here. The map's
    // signatures start with an ObjPtr const&.
    virtual void _InitDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) {
        static_cast<void>(instances);
    }
    virtual void _SyncDrawInstancesImpl(VectorAdapter<RndDrawInstance>& instances) {
        static_cast<void>(instances);
    }
    // Slot 45 at 0x44ADA0, shared with the subclasses: true here and in
    // every subclass but RndDecalCom. No caller was found, so the name is
    // weak. Name not in the reference map.
    virtual bool _DrawsGeometry() const;

    // Stores the instance lists of every level, marks the instances shown
    // when the entity is active and initializes and syncs them. The map's
    // signature starts with an ObjPtr const&.
    void InitDrawInstances(VectorAdapter<RndDrawInstance>* instances);  // 0x6C1610
    // Syncs every level's non-empty instance list (slot 44). The map's
    // signature starts with an ObjPtr const&.
    void _SyncDrawInstances();  // 0x6C1720
    // Sets one of the two clip planes of every instance. No caller in this
    // build. Name not in the reference map.
    void SetInstanceClipPlane(unsigned long index, const Vector4& plane);  // 0x6C1780
    // Leaves the drawer the component registered with and registers with
    // the given one when any level has instances. No caller in this build.
    // The map's signatures start with an ObjPtr const&.
    void _RegisterWithSceneDrawer(RndSceneDrawer* drawer);    // 0x6C3070
    void _DeRegisterWithSceneDrawer(RndSceneDrawer* drawer);  // 0x6C31F0
    // Re-registers an entered component with its scene drawer, for a
    // subclass whose instance counts changed (0x6721B4). Name not in the
    // reference map.
    void _ReRegisterWithSceneDrawer();  // 0x6C3620
    // Stores the state flags of the instances: the material's usage hints
    // for the quality level and the draw node's flag 0x200 as 0x40000. The
    // material is the object's, the referenced object's, or the default.
    // Called by the subclasses' slots 43 and 44. Name not in the reference
    // map.
    void _SyncInstanceStateFlags(VectorAdapter<RndDrawInstance>& instances);  // 0x6C3500

    // The number of scene levels: three with RndConfig::mUseLod, else one.
    // Inlined everywhere. Name not in the reference map.
    static unsigned long NumSceneLods();

    // Registers the class description and the properties. Not
    // reconstructed: the property metadata's attributes are written through
    // helpers that are not modelled.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x6C17F0

    static Symbol sId;          // 0x1AB0370, "DrawInstance"
    // The class symbol GameObject::CreateComponent takes; also
    // "DrawInstance". Name not in the reference map.
    static Symbol sClassName;   // 0x1AB0378
    static PropRegistry sPropRegistry;  // 0x1AB0380
    static ComMetaData sMetaData;       // 0x1AB0420

    // "billboarding": how to billboard when drawing the object.
    int mBillboarding;
    // "sort_by".
    int mSortBy;
    // "sorting_hint".
    bool mSortingHint;
    // "lods": the scene levels the object is drawn at, all three by default.
    unsigned int mLods;
    // "extra_data" and "extra_data_1", copied into every instance.
    Vector4 mExtraData;
    Vector4 mExtraData1;
    RuntimeData mRuntime;
};

static_assert(offsetof(RndDrawInstanceCom, mBillboarding) == 24);
static_assert(offsetof(RndDrawInstanceCom, mSortBy) == 28);
static_assert(offsetof(RndDrawInstanceCom, mSortingHint) == 32);
static_assert(offsetof(RndDrawInstanceCom, mLods) == 36);
static_assert(offsetof(RndDrawInstanceCom, mExtraData) == 40);
static_assert(offsetof(RndDrawInstanceCom, mExtraData1) == 56);
static_assert(offsetof(RndDrawInstanceCom, mRuntime) == 72);
static_assert(offsetof(RndDrawInstanceCom::RuntimeData, mRegistered) == 1);
static_assert(offsetof(RndDrawInstanceCom::RuntimeData, mSortKey) == 4);
static_assert(offsetof(RndDrawInstanceCom::RuntimeData, mActiveSortKey) == 8);
static_assert(offsetof(RndDrawInstanceCom::RuntimeData, mDrawInstances) == 16);
static_assert(offsetof(RndDrawInstanceCom::RuntimeData, mTrans) == 64);
static_assert(offsetof(RndDrawInstanceCom::RuntimeData, mDefaultMaterial) == 96);
static_assert(offsetof(RndDrawInstanceCom::RuntimeData, mSceneDrawerLink) == 104);
static_assert(sizeof(RndDrawInstanceCom) == 192);
