#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/DynamicCom.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRegistry.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

class DataArray;
class Entity;
class EntityHeaderCom;
class MemStream;

// The source of the properties an InterfaceCom ("Component for viewing
// properties exposed by an instance", 0x1DA250) views: the component that
// instances an entity, whose root exposes them. A seven-slot interface
// without a destructor; InstanceCom places it at +64. Its out-of-line
// members are emitted at 0x12B450-0x12BCCF, in an object the map does not
// name. Name and slot names not in the reference map; the evidence is
// weak.
class InterfaceSource {
public:
    // Slot 0: the instanced entity.
    virtual Entity* GetInterfaceEntity() = 0;
    // Slot 1: the component that is the source (InterfaceSourceImpl).
    virtual Component* GetInterfaceComponent() = 0;
    // Slot 2: the resource the entity comes from, or null when it is
    // missing or failed.
    virtual TransEntityResource* GetInterfaceResource() = 0;
    // Slot 3: stored by the InterfaceCom when it binds (+200); while it is
    // set, the InterfaceCom forwards inserts, removals and messages to the
    // source entity's component.
    virtual bool _ForwardsInterfaceEdits() = 0;
    // Slot 4: the path of an interface property in the source; the path
    // itself here.
    virtual PropPath _MapInterfacePath(const PropPath& path);  // 0x12B470
    // Slot 5: gives the InterfaceCom the properties the source stored for
    // it.
    virtual void _TransferStoredProps(Component& com) = 0;
    // Slot 6: copies the InterfaceCom's properties into the source entity's
    // component and sends that component "post_load_resources".
    virtual void _PostLoadInterface(Component& com) = 0;

    // Destroys the object's interface components when the component alone
    // is destroyed (kDestroyComponent). Not reconstructed.
    void _PreDestroyInterfaces(Component* com, DestroyType type);  // 0x12BC10
    // Creates, binds and removes the object's interface components for the
    // properties the instanced entity's root exposes. Not reconstructed.
    void _SyncInterfaces(Component* com);  // 0x12B720
};

// Gives InterfaceSource's slot 1 to the class T that derives from it, as
// T itself. InstanceCom's copy is at 0x11B880 and is not overridden by
// its subclasses. Name not in the reference map.
template <class T>
class InterfaceSourceImpl : public InterfaceSource {
public:
    Component* GetInterfaceComponent() override {
        return static_cast<T*>(this);
    }
};

// The component that instances an entity resource on its object
// (entity/InstanceCom.o, 0x118DC0 to 0x122DA0): it loads the
// TransEntityResource named by "file" and either shares the resource's
// entity or, when the entity's EntityHeaderCom asks for "copy_on_instance",
// copies it (Entity::Copy). The child entity is parented to the object,
// entered, exited and polled with it, and may drive the object's
// transform. Its class id is "Instance" ("Used to load an entity
// resource"); its metadata names it "InstanceCom". The vtable at 0x18E6D58
// has 49 slots: Component's 41, three of the class's own and the five
// InterfaceSource overrides; the DynamicComResource vtable at +24 is
// 0x18E6EF0 and the InterfaceSource vtable at +64 is 0x18E6F08. The object
// is 128 bytes. Field names are not in the reference map.
class InstanceCom : public ResourceCom<TransEntityResource>, public InterfaceSourceImpl<InstanceCom> {
public:
    // The instancing state. The map names a RuntimeData type; the field
    // names are not in the reference map.
    struct RuntimeData {
        RuntimeData();  // 0x1196E0

        // The child entity: the resource's own when it is shared, or a
        // copy.
        Entity* mEntity;
        // Storage the next copy of the entity is built in, taken by
        // _InstanceChild; its writer is not identified.
        void* mCopyBuffer;
        // The properties revision 1 of _PostLoad read, and the entity
        // revision they were read under; _TransferStoredProps hands them to
        // an InterfaceCom.
        MemStream* mStoredProps;
        unsigned char mPadding[8];  // Never read or written in this object.
        int mStoredPropsRev;
        // The revision the registry's revision handler (0x11BF10) records
        // when the properties load; 1 until then.
        int mLoadedRev;
        // _CanBeDrivenByChild, cached when the resources load.
        bool mCanBeDrivenByChild;
        // Set while the child has its poll dependency on the parent
        // (_PostLoadResources), cleared when it exits.
        bool mParentPollDepAdded;
        // Set when the child entity changed; the poll clears it.
        bool mNeedsInstance;
    };

    // Inlined into the subclasses' constructors and the registry's
    // prototypes (0x118E60).
    InstanceCom() : mActive(true), mPollEnabled(true), mRuntime() {}
    // Copies the properties for an imprint; the resources and the run-time
    // state start afresh. Inlined into _Imprint (0x11B630).
    InstanceCom(const InstanceCom& other)
        : ResourceCom<TransEntityResource>(other),
          InterfaceSourceImpl<InstanceCom>(),
          mActive(other.mActive),
          mPollEnabled(other.mPollEnabled),
          mRuntime() {}

    // Slots 0-1: 0x119710, 0x119790. Frees the stored properties.
    ~InstanceCom() override;
    // Slot 2: "set_active", "get_active", "get_entity" and "using_child";
    // any other message is unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x11B330
    Symbol GetId() const override;          // slot 4: 0x11B5B0
    Symbol GetClassName() const override;   // slot 5: 0x11B5C0
    int CurrentRev() const override;        // slot 7: 0x11B5D0
    bool IsA(Symbol type) const override;   // slot 8: 0x11B5F0
    Component* AsComponent() override;      // slot 9: 0x11B620
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x11B630
    // Slot 15 at 0x119870: drops the stored properties and reads their
    // format: revision 1 holds them, revision 2 nothing.
    void _PostLoad(BinStream& stream) override;
    // Slot 16 at 0x119810: the properties, then revision 2.
    void _Save(BinStream& stream) override;
    // Slot 18 at 0x119990: the object's TransCom follows the instance, or
    // precedes it when the child drives the object.
    void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes) override;
    // Slot 20 at 0x119B90: destroys the interface components, and a copied
    // child; a shared child is unparented. The map's _PreDestroy(ObjPtr
    // const&, DestroyType).
    void _PreDestroy(DestroyType type) override;
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x11B7D0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x11B7E0
    // Slot 25 at 0x11A620: lets the child entity prepare its components
    // when the thread enters entities at once. The map's
    // _PreLoadResources(ObjPtr const&).
    void _PreLoadResources() override;
    // Slot 26 at 0x11A6E0: when the thread enters entities at once, makes
    // the child poll after the dependency (`context`, or the object's
    // entity) and, when it drives the object, the parent poll after the
    // child; `loaded` polls the child at once. The map's
    // _PostLoadResources(ObjPtr const&, void*).
    void _PostLoadResources(void* context, bool loaded) override;
    // Slot 29 at 0x119D20: reloads the resource and instances the child.
    // The map's _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;
    // Slot 30 at 0x11A8B0: the child's resources are ready.
    bool _AreResourcesReady(int pass) override;
    // Slot 31 at 0x11A8D0: parents a shared child, takes the child's
    // transform when it drives the object, and enters the child unless it
    // is a shared child already entered elsewhere.
    void _Enter() override;
    // Slot 32 at 0x11ABE0: drops the poll dependencies when the object or
    // the component is destroyed, and exits a copied child or one that
    // re-enters.
    void _Exit(DestroyType type) override;
    // Slot 33 at 0x11AE80.
    void _Poll() override;
    // Slots 34-35 at 0x11B090 and 0x11B0B0: deactivate the child, or give
    // it "active".
    void _OnDeactivate() override;
    void _OnActivate() override;
    // Slots 36-38 at 0x11AD80, 0x11AE60 and 0x11B0D0: enter, exit and poll
    // the child in the edit mode.
    void _EditEnter() override;
    void _EditExit(DestroyType type) override;
    void _EditPoll() override;

    // Slot 41 at 0x11B810: whether a shared child enters and exits again
    // with each instance. True here. The map's _ReenterSharedChildren()
    // const.
    virtual bool _ReenterSharedChildren() const;
    // Slot 42 at 0x11B820: whether the child polls after the object's
    // entity. True here. The map's _CanHaveParentPollDep() const, a name
    // Component's slot 28 holds in this reconstruction.
    virtual bool _HasParentPollDep() const;
    // Slot 43 at 0x11B830: whether the child may drive the object's
    // transform. True here. Name not in the reference map.
    virtual bool _CanBeDrivenByChild() const;
    // Slots 44-48 override InterfaceSource's slots 2, 0, 3, 5 and 6; their
    // copies at +64 are 0x11A1C0, 0x11B870, 0x11B890 and the thunks
    // 0x11A350 and 0x11A610.
    TransEntityResource* GetInterfaceResource() override;  // slot 44: 0x11A190
    Entity* GetInterfaceEntity() override;                 // slot 45: 0x11B840
    bool _ForwardsInterfaceEdits() override;               // slot 46: 0x11B850
    // Slot 47. Not reconstructed: it moves MemStream's members directly.
    void _TransferStoredProps(Component& com) override;  // 0x11A1F0
    // Slot 48. Not reconstructed: the property values are copied through
    // the registry walk at 0x11C220, which is not modelled.
    void _PostLoadInterface(Component& com) override;  // 0x11A360

    // Whether the child's root drives the object's transform. The map's
    // IsDrivenByChild() const.
    bool IsDrivenByChild() const;  // 0x119B40
    // Whether the child is a copy: the resource is missing or failed, or
    // its entity asks for "copy_on_instance". The map's _GetCopyChild()
    // const.
    bool _GetCopyChild() const;  // 0x119C90
    // The bytes a copy of the child takes, rounded to 8, or 0 when the
    // child is shared. The map's _ChildSize().
    unsigned long _ChildSize() const;  // 0x11A0D0
    // The poll's work, shared with the edit poll: polls an active child
    // (under the parent's poll manager unless the thread enters entities
    // at once), takes its transform when it drives the object, and lets a
    // lightweight object release the child's poll job early. Name not in
    // the reference map.
    void _PollChild();  // 0x11AE90
    // The "using_child" handler: runs the message's script from node 2
    // with the child as the default entity and its root as the object.
    // The map's _OnUsingChild(DataArray*).
    DataNode _OnUsingChild(DataArray* msg);  // 0x11B110

    // Drops the child when the resource changes, reloads the resource and
    // checks the class CRC of the component order. Name not in the
    // reference map.
    bool _LoadChildResource();  // 0x119D50
    // Shares or copies the resource's entity as the child, parents it,
    // takes its transform when it drives the object and syncs the
    // interface components. Name not in the reference map.
    void _InstanceChild();  // 0x119E90

    // Describes the class, registers the resource properties and, for the
    // class's own registry, "active", "poll_enabled", the file property's
    // resource class and the revision handler.
    static void _Init(PropRegistry& registry, ComMetaData& metadata, const char* fileProp);  // 0x118E60
    // Registers the resource file under `fileProp`, "reloads_self" and
    // "com_order_crc". The map's signature is
    // _InitPropRegistry(PropRegistry&).
    static void _InitPropRegistry(PropRegistry& registry, const char* fileProp);  // 0x11B8E0
    // The class's registration, with the file property "file".
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x1196D0
    // Makes InstanceCom the superclass of a subclass's metadata, runs
    // _Init on it in the "metadata" heap and combines the editor
    // restrictions. Name not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata, const char* fileProp);  // 0x118DC0

    // "Instance", constructed by the static initializer at 0x122D30.
    static Symbol sId;  // 0x19E3278
    // Also "Instance"; the class symbol GameObject::CreateComponent and the
    // component-order lists take, and EntityResource::GetInstanceComId
    // returns. Name not in the reference map.
    static Symbol sClassName;           // 0x19E3280
    static PropRegistry sPropRegistry;  // 0x19E3290
    static ComMetaData sMetaData;       // 0x19E3330

    // "active": the child entity is active (Entity::SetActive).
    bool mActive;
    // "poll_enabled": the child entity polls (Entity::SetPollEnabled).
    bool mPollEnabled;
    RuntimeData mRuntime;
};

static_assert(offsetof(InstanceCom::RuntimeData, mStoredPropsRev) == 32);
static_assert(offsetof(InstanceCom::RuntimeData, mCanBeDrivenByChild) == 40);
static_assert(offsetof(InstanceCom::RuntimeData, mNeedsInstance) == 42);
static_assert(sizeof(InstanceCom::RuntimeData) == 48);
static_assert(offsetof(InstanceCom, mResource) == 32);
static_assert(offsetof(InstanceCom, mInlineResource) == 40);
static_assert(offsetof(InstanceCom, mFile) == 48);
static_assert(offsetof(InstanceCom, mComOrderCrc) == 56);
static_assert(offsetof(InstanceCom, mReloadsSelf) == 60);
static_assert(offsetof(InstanceCom, mActive) == 72);
static_assert(offsetof(InstanceCom, mPollEnabled) == 73);
static_assert(offsetof(InstanceCom, mRuntime) == 80);
static_assert(sizeof(InstanceCom) == 128);
