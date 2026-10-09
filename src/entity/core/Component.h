#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

class BinStream;
class ComMetaData;
class DataArray;
class Entity;
class PropPath;
class PropRegistry;
class TextStream;

// Why a component exits or is destroyed. The map names the type; the
// enumerator names are not in the reference map. Only the values the
// reconstructed code passes are declared.
enum DestroyType : int {
    // No destroy is in progress on the thread (Entity.o's thread-local
    // gDestroyType starts here).
    kDestroyUnset = -1,
    // A top-level entity exits or is destroyed (Entity::Exit, 0xF50D0).
    kDestroyRoot = 0,
    // An instanced entity whose resource is an editor entity (bit 11 of the
    // entity's flags); the evidence for the name is weak.
    kDestroyEditorInstance = 1,
    // An instanced entity. A nested entity keeps the lowest type of the
    // entities around it.
    kDestroyInstance = 2,
    // The object exits or is destroyed with its components (0x116890).
    kDestroyObject = 3,
    // The component alone is destroyed, or exits to reload its resources
    // (0x117760, 0xE80F0).
    kDestroyComponent = 4,
};

// The serializer state the prop storage slots (12 and 13) receive from the
// object serializer at 0x106260 and the reader at 0x106850. Not modelled.
// Name not in the reference map.
struct PropStorageArgs;

// The base of every entity component (entity/Component.o, 0xE7190-0xEA1FF).
// A component belongs to one GameObject and is created by its class's
// factory (sFactory). Its vtable at 0x18E6518 has 41 slots; the eight pure
// slots are each class's identity, generated for it by the component
// macros. Entities enter, poll and exit their components in one of two
// modes: the game mode (slots 31-33) and the edit mode (slots 36-38).
// Names in the reference map are kept; the others are inferred from the
// bodies and their callers.
class Component {
public:
    Component();  // 0xE7190
    // Copies nothing but the vtable and the defaults; _Imprint uses it.
    Component(const Component& other);  // 0xE71C0

    // Slots 0-1: 0xE71F0, 0xE7200.
    virtual ~Component();
    // Slot 2: handles "load_resources", "size", "remove" and "insert".
    virtual DataNode Handle(DataArray* msg, bool warn);  // 0xE8670
    // Slot 3: the owning object's entity, or null. Name not in the
    // reference map.
    virtual Entity* GetEntity() const;  // 0xAEA0
    // Slot 4: the class's sId.
    virtual Symbol GetId() const = 0;
    // Slot 5: the class's sClassName. Name not in the reference map.
    virtual Symbol GetClassName() const = 0;
    // Slot 6: the class metadata's interface. Name not in the reference
    // map.
    virtual Symbol GetInterfaceId() const;  // 0xAED0
    // Slot 7: the property registry's current revision.
    virtual int CurrentRev() const = 0;
    // Slot 8: whether the class is the type or derives from it.
    virtual bool IsA(Symbol type) const = 0;
    // Slot 9: the component itself; every class returns this. Name not in
    // the reference map; the evidence is weak.
    virtual Component* AsComponent() = 0;
    // Slot 10: copy-constructs the component at the next 8-byte boundary of
    // the buffer, stores it in `imprint` when that is given, and returns the
    // end of the copy's properties (_ImprintProps). The map's signature is
    // _Imprint(char*, Component*&, bool).
    virtual char* _Imprint(char* buffer, Component** imprint) = 0;
    // Slot 11: copies the dynamic properties into the buffer for the
    // imprint, which may be null to size the copy. The map's signature is
    // _ImprintProps(Component*, char*); this build swaps the arguments.
    virtual char* _ImprintProps(char* buffer, Component* imprint);  // 0xE8D30
    // Slots 12-13: write the component's class and properties to the prop
    // storage and read them back; the read marks the component imprinted.
    // Names not in the reference map; the evidence is weak. Not
    // reconstructed.
    virtual void _SaveStorage(PropStorageArgs& args);  // 0x107220
    virtual void _LoadStorage(PropStorageArgs& args);  // 0x107300
    // Slot 14 at 0xB230: reset after the entity loads (Entity::ResetEntered
    // at 0xF38E0); empty here.
    virtual void _ResetEntered() {}
    // Slot 15 at 0xB240: called by _Load after the properties are read;
    // empty here.
    virtual void _PostLoad(BinStream& stream) {
        static_cast<void>(stream);
    }
    // Slot 16: writes the component's revision and properties.
    virtual void _Save(BinStream& stream);  // 0xE8C30
    // Slots 17-18: the component classes this class's components follow
    // and precede, in the poll order (slot 17) and in the component order
    // (slot 18). By default every component follows TransCom. Names not in
    // the reference map; the evidence is weak.
    virtual void _GetPollOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes);  // 0xE9060
    virtual void _GetComponentOrderDeps(
        eastl::vector<Symbol>& follows,
        eastl::vector<Symbol>& precedes);  // 0xE8FA0
    // Slot 19 at 0xB250: called once the component is created and the
    // entity resource is told; empty here.
    virtual void _PostCreate() {}
    // Slot 20 at 0xB260: called before the component is destroyed; empty
    // here.
    virtual void _PreDestroy(DestroyType type) {
        static_cast<void>(type);
    }
    // Slot 21 at 0xB270: loads the component's resources; false when one
    // failed. False here.
    virtual bool _LoadResources() {
        return false;
    }
    // Slots 22-23: the class's property registry and metadata.
    virtual PropRegistry& _GetPropRegistry() = 0;
    virtual ComMetaData& _GetMetaData() = 0;
    // Slot 24 at 0xB2A0: called after the prop storage is read; empty here
    // and in every class. Name not in the reference map.
    virtual void _PostLoadStorage() {}
    // Slots 25-26 at 0xB2B0 and 0xB2C0: around the entity's resource load;
    // GameObject calls slot 26 with a null context and true. Empty here.
    // The map has them on InstanceCom; this build's signatures differ.
    virtual void _PreLoadResources() {}
    virtual void _PostLoadResources(void* context, bool loaded) {
        static_cast<void>(context);
        static_cast<void>(loaded);
    }
    // Slot 27 at 0xB2D0: the objects that must poll before and after this
    // component's object; empty here. The map's signature starts with an
    // EntityPtr.
    virtual void _GetPollDeps(
        eastl::vector<GameObjectId>& before,
        eastl::vector<GameObjectId>& after) {
        static_cast<void>(before);
        static_cast<void>(after);
    }
    // Slot 28 at 0xB2E0: lets the object poll after its parent entity's
    // object. False here; one class returns true. The map has it on
    // InstanceCom only; the evidence is weak.
    virtual bool _CanHaveParentPollDep() const {
        return false;
    }
    // Slot 29 at 0x18EE0: checks the component after its resources load;
    // false keeps the component from re-entering. True here. Name not in
    // the reference map.
    virtual bool _OnResourcesLoaded() {
        return true;
    }
    // Slot 30 at 0xB2F0: whether the resources finished loading;
    // GameObject asks until every component is ready. True here. Name not
    // in the reference map.
    virtual bool _AreResourcesReady() {
        return true;
    }
    // Slots 31-33 at 0x401C0, 0x39E30 and 0x401D0: enter, exit and poll in
    // the game mode; empty here. _Exit is not in the reference map.
    virtual void _Enter() {}
    virtual void _Exit(DestroyType type) {
        static_cast<void>(type);
    }
    virtual void _Poll() {}
    // Slots 34-35 at 0xB300 and 0xB310: the entity was deactivated or
    // activated (Entity's active flag, 0xF3140); empty here. Names not in
    // the reference map.
    virtual void _OnDeactivate() {}
    virtual void _OnActivate() {}
    // Slots 36-38 at 0xB320, 0xB330 and 0xB340: enter, exit and poll in the
    // edit mode; empty here. _EditEnter and _EditExit are not in the
    // reference map.
    virtual void _EditEnter() {}
    virtual void _EditExit(DestroyType type) {
        static_cast<void>(type);
    }
    virtual void _EditPoll() {}
    // Slots 39-40 at 0xB350 and 0xB360: whether the component post-polls,
    // and the post-poll. False and empty here.
    virtual bool _HasPostPoll() const {
        return false;
    }
    virtual void _PostPoll() {}

    // Whether a factory is registered for the class.
    static bool IsValid(Symbol id);  // 0xE7220
    // The hash of the class's poll-order and component-order dependencies,
    // which ComMetaData::Init folds into the class CRC: a component of the
    // class is created through its factory to answer and destroyed. The
    // FNV-1a basis for a class without a factory. Name not in the
    // reference map.
    static unsigned int GetClassCRC(Symbol id);  // 0xE72A0
    // The FNV-1a hash of the four dependency lists (slots 17 and 18), each
    // sorted without regard to case, with '-' between them; the basis when they
    // are all empty. Name not in the reference map.
    unsigned int _GetOrderDepsCRC();  // 0xE7380
    // Destroys the component, freeing it unless it lives in an imprint.
    // Name not in the reference map.
    void Destroy();  // 0xE7210
    // The element count of the array property at the path, or 0.
    int Size(const PropPath& path) const;  // 0xE78E0
    // Inserts a default element into the array property at the path.
    bool Insert(const PropPath& path);  // 0xE7AC0
    // Inserts a value of the type into the array property at the path; the
    // last node is the index. True when the array grew.
    bool _Insert(const PropPath& path, const void* value, PropertyType type);  // 0xE7AD0
    // Removes the element the path names. True when the array shrank.
    bool Remove(const PropPath& path);  // 0xE7E50
    // Loads the component's resources, exiting and re-entering an entered
    // component around them. `quiet` suppresses the failure report. The
    // map's signature is LoadResources(ObjPtr const&, bool); the return
    // value is whether the component may re-enter.
    bool LoadResources(bool quiet);  // 0xE80F0
    // Whether the component's resources are ready, asking slot 30 until
    // they are. Name not in the reference map.
    bool AreResourcesReady();  // 0xE8250
    // A description for error messages, "%s component in %s" from the class
    // id and the object's MakeErrorName. The map's MakeErrorName(ObjPtr
    // const&) const; this build uses the owning object.
    const char* MakeErrorName() const;  // 0xE8280
    // Writes and reads the values of the properties under the path.
    bool StorePropState(const PropPath& path, BinStream& stream) const;  // 0xE8310
    bool ApplyPropState(const PropPath& path, BinStream& stream);       // 0xE8320
    // Prints the properties to the stream, or to TheDebug when it is null.
    // The map's signature starts with an ObjPtr const&.
    void Dump(TextStream* stream, bool verbose) const;  // 0xE8330
    // The "remove" and "insert" handlers: the message's nodes from 2 on
    // are the path.
    DataNode _OnRemove(DataArray* msg);  // 0xE84C0
    DataNode _OnInsert(DataArray* msg);  // 0xE8590
    // The revision _Save writes. Name not in the reference map.
    static int SaveRev();  // 0xE8C20
    // Reads what _Save wrote into the component; a revision below 2 has no
    // properties. The map's signature is _Load(ObjPtr const&, Component*,
    // BinStream&).
    static void _Load(Component* component, BinStream& stream);  // 0xE8CA0

    // Copies the properties of the registry into the buffer, for the
    // imprint's registry when there is an imprint, and returns the end of
    // the copies. Name not in the reference map. Not reconstructed.
    char* _ImprintRegistry(
        char* buffer,
        Component* imprint,
        const PropRegistry& registry,
        const PropRegistry* imprintRegistry);  // 0xE8D90

    // The class factories by class id; GameObject::_CreateComponent creates
    // through them.
    static eastl::map<Symbol, Component* (*)()> sFactory;  // 0x19E2900
    // Lets a class registered for the root object only be created on any
    // object (GameObject::_CreateComponent).
    static bool sRegressionTesting;  // 0x19E2970

    // Field names are not in the reference map.
    // The owning object, set by GameObject::_CreateComponent (0x116AE0).
    GameObject* mObject;
    // Set for a component that lives in an imprint or in prop storage; it is
    // destroyed without being freed.
    bool mImprinted;
    // Set by the constructor; one subclass clears it while it rebuilds its
    // properties and sets it after (0x1D10C0, 0x1D0EE0). Weak evidence.
    bool mPropsSynced;
    // Set once LoadResources has run.
    bool mResourcesRequested;
    // Set once slot 30 reported the resources ready.
    bool mResourcesReady;
    // Set while the component is entered, in either mode.
    bool mEntered;
    // Cleared by the constructor; no reader is identified.
    bool mReserved;
    // The constructor stores the four bytes from mResourcesRequested to
    // mReserved together. Subclasses place their first member in the tail
    // padding at 22: AudioEmitterCom's "is_named_emitter", the "Options"
    // base of RndOverlayOptionsCom (0x46A870), and the flag RndDefaults
    // writes on RndLightCom and RndLightProbeCom (0x6BEF40, 0x6BFA60).
};

static_assert(offsetof(Component, mObject) == 8);
static_assert(offsetof(Component, mImprinted) == 16);
static_assert(offsetof(Component, mEntered) == 20);
static_assert(offsetof(Component, mReserved) == 21);
static_assert(sizeof(Component) == 24);

// Visits the component's saved resource-path and property-reference
// properties (types 18 and 19), recursing into arrays and structs; called by
// Component::LoadResources unless it is quiet. Name not in the reference
// map; the evidence is weak. Not reconstructed.
void LoadPropResources(Component* component);  // 0x1C9A90
