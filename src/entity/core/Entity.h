#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/resources/Resource.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"
#include "utl/threading/PollDep.h"

class BinStream;
class EntityResource;

// A set of game objects grouped into layers (entity/Entity.o). An entity is
// an event source and a poll job: its vtable at 0x18E6818 has MsgSource's
// seven slots and five more for the PollDepBase overrides, and the
// PollDepBase vtable at 0x18E6888 (+24) has ten. The 320-byte object is
// built by the constructor at 0xEBB70.
class Entity : public MsgSource, public PollDepBase {
public:
    // One layer's objects, held in the layer's PropArray<GameObject*>. The
    // map names the type; the field names are not in the reference map.
    struct Layer {
        PropArray<GameObject*> mObjects;
        // Counts the objects created in the layer;
        // _CreateAndInsertNewGameObject puts it in the top 16 bits of each
        // new GameObjectId.
        unsigned short mNextSerial;
        unsigned char mPadding[6];  // Never read or written.

        // The steps loading the layer takes: its objects and the resources
        // inlined into it. Name not in the reference map.
        unsigned int GetLoadStepCount(unsigned long layer, const Entity* entity) const;  // 0xF7550
        // Places the layer's object pointers at the buffer for the imprint,
        // which may be null to size the copy, then imprints each object for
        // the entity's copy. Returns the end of the copies. The map has
        // Imprint(Entity::Layer*, char*) and ImprintSize(); this build
        // sizes with a null imprint and passes the entity's copy.
        char* Imprint(char* buffer, Entity* entity, Layer* imprint) const;  // 0xF68C0
    };

    // Builds an active entity with the given number of empty layers. The
    // map's signature is Entity(); this build passes the layer count.
    explicit Entity(unsigned long numLayers);  // 0xEBB70
    // Copies the entity for an imprint: the arrays stay empty, since the
    // thread is imprinting, and the resource, parent, flags and tag are
    // copied. Inlined into _Imprint (0xEE3F0).
    Entity(const Entity& other);
    // Slots 0-1: 0xEBCC0, 0xEBED0 (thunks 0xEBEC0, 0xEBEF0).
    ~Entity() override;
    // Slot 2: the entity's script messages ("set_active", "save_file",
    // "create_instance", "export" and others). Not reconstructed.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xF6DB0
    // Slot 3: the entity itself.
    void* GetSinkObject() override;  // 0xF7910
    // Slot 7 (PollDepBase slot 3, thunk 0xF5980): polls the entity through
    // its resource. The map's signature is ThreadPoll().
    void ThreadPoll(const int& thread) override;  // 0xF5960
    // Slot 8 (PollDepBase slot 4, thunk 0xF7990): post-polls an active
    // entity.
    void PostPoll(const int& thread) override;  // 0xF7920
    // Slot 9 (PollDepBase slot 2, thunk 0xF59A0): whether the entity is
    // active and entered without a pending exit, and, when it drives its
    // parent (flag 0x8000), whether its parent may poll.
    bool IsPollEnabled() const override;  // 0xF57C0
    // Slots 10-11 (PollDepBase slots 6-7, thunks 0xF79C0 and 0xF79F0): an
    // entity entered in the game mode tells its resource it is ready.
    void _OnAddPollDep() override;     // 0xF7930
    void _OnRemovePollDep() override;  // 0xF7960

    // Copies the entity into the buffer, or into a new allocation when it
    // is null; a copy built in place gains flag 0x200. The copy is then
    // parented (SetParent), and `loadResources` loads its resources
    // quietly. The map's signature is Copy(EntityPtr const&, void*, bool,
    // ObjPtr const&, Symbol, DestroyType); this build copies `this` and
    // passes no destroy type. InstanceCom instances its child with it.
    Entity* Copy(void* buffer, bool loadResources, GameObject* parent, Symbol parentComId) const;  // 0xEE270
    // Copy, but a null parent keeps the copied one. Name not in the
    // reference map.
    Entity* Clone(void* buffer, bool loadResources, GameObject* parent, Symbol parentComId) const;  // 0xEE330
    // Makes the object, whose component of the class instances the entity,
    // the parent; a change sets flag 0x4000. The map's signature is
    // SetParent(ObjPtr const&, Symbol, DestroyType).
    void SetParent(GameObject* parent, Symbol parentComId);  // 0xEE850
    // Builds the copy in the buffer: the entity, its poll orders, its
    // layers and every object with its components, each part aligned for
    // its type. A null buffer only sizes the copy. Returns the end of the
    // copy rounded to 16 bytes. The imprint lock is held throughout. The
    // map's _Imprint(char*).
    char* _Imprint(char* buffer) const;  // 0xEE3F0
    // Creates an object in the layer with room for the given number of
    // components, and lets the resource initialize it. The map's signature
    // is CreateObject(unsigned long); this build adds the component
    // reserve.
    GameObject* CreateObject(unsigned long layer, unsigned long numComs);  // 0xF0A10
    // The object with the name, or null. The failure report is made only
    // when fail is set.
    GameObject* TryGetObject(Symbol name, bool fail) const;  // 0xF0FF0
    // The object with the id, or null when the id is invalid or stale. The
    // failure report that `fail` asks for is compiled out; only the
    // entity's description is made.
    GameObject* SafeGetObject(GameObjectId id, bool fail) const;  // 0xF11C0
    // Destroys the object, through the list form at 0xF1580.
    void DestroyObject(GameObjectId id);  // 0xF1530
    // The first object of any layer. The map's signature is
    // BeginObject() const; this build forwards to NextObject.
    GameObject* BeginObject() const;  // 0xEF180
    // The object after the given one that holds a component with the class
    // or base-class symbol; the empty symbol matches any object. The map's
    // signature is NextObject(GameObjectId, Symbol) const; this build takes
    // the object.
    GameObject* NextObject(const GameObject* object, Symbol com) const;  // 0xEF1A0
    // The number of objects in all layers. Not reconstructed. Name not in
    // the reference map.
    unsigned long NumObjects() const;  // 0xF1350
    // A description for error messages: the resource's id and path, and
    // the resource it is inlined in.
    const char* MakeErrorName() const;  // 0xF0DD0
    // Forgets the parent object; a change sets flag 0x4000. Name not in
    // the reference map.
    void ClearParent();  // 0xF0740
    // The entity whose instance object holds this one, or null.
    Entity* GetParent() const;  // 0xF0780
    // The outermost entity: the one whose ancestors hold this one, or this
    // entity when it has no parent. Name not in the reference map.
    Entity* GetTopEntity();  // 0xF09E0
    // Whether the entity resource has the layer; the main layer always
    // exists. Forwards to the resource. The map's LayerExists(unsigned
    // long) const.
    bool LayerExists(unsigned long layer) const;  // 0xF25B0
    // The layer's file. Forwards to the resource. The map's
    // GetLayerPath(unsigned long) const.
    ResourcePath GetLayerPath(unsigned long layer) const;  // 0xF3050
    // Activates or deactivates the entity (flag 0x100) and tells the
    // components (Component slots 34-35); InstanceCom's "active" and
    // "set_active" set it. Name not in the reference map. Not
    // reconstructed.
    void SetActive(bool active);  // 0xF3140
    // Enables or disables polling (flag 0x40); enabling marks the root's
    // TransCom dirty. InstanceCom's "poll_enabled" sets it. Name not in the
    // reference map.
    void SetPollEnabled(bool enabled);  // 0xF3630
    // Whether every object's components are ready on the load pass, asking
    // each object even after one is not. Name not in the reference map.
    bool AreResourcesReady(int pass);  // 0xF39C0

    // Assigns the next id in the layer and stores the object there,
    // allocating a new one when `object` is null. Returns the id.
    GameObjectId _CreateAndInsertNewGameObject(unsigned long layer, GameObject* object);  // 0xF0AD0
    // Loads the resources of every object's components, then waits (up to
    // ten passes) for them to be ready unless the parent entity is still
    // loading. `quiet` suppresses the components' failure reports.
    bool _LoadResources(bool quiet);  // 0xEE880
    // Reads the entity's root from the stream: the revision, the layer
    // files, the root data, the layer count and the root object. The map's
    // signature is _LoadRoot(BinStream&, EntityPtr, vector<unsigned char>&,
    // vector<ResourcePath>&); this build passes the paths first, and the
    // root data may be null.
    // Read and write the entity, its root data and its objects. Not
    // reconstructed.
    static Entity* _LoadCached(BinStream& stream, EntityResource* resource);    // 0xEEAF0
    static Entity* _LoadUncached(BinStream& stream, EntityResource* resource);  // 0xEF2D0
    void _SaveCached(BinStream& stream);    // 0xEFF10
    void _SaveUncached(BinStream& stream);  // 0xF06B0
    static void _LoadRoot(
        BinStream& stream,
        Entity* entity,
        eastl::vector<ResourcePath>* paths,
        eastl::vector<unsigned char>* rootData);  // 0xEFA00
    // Rebuilds the entity's poll order when it is marked stale (flag
    // 0x400). The map's _UpdatePollOrder(). Not reconstructed.
    void _UpdatePollOrder();  // 0xF0260
    // Calls _ResetEntered on every component and marks the objects' poll
    // orders stale. The map's ResetEntered(). Not reconstructed.
    void ResetEntered();  // 0xF38E0
    // Points the entity's references to the object's component of the class
    // at another object, or at none. The map's signature is
    // _ReplaceObject(GameObjectId, GameObjectId, Entity::ReplaceType,
    // EntityPtr, set*, set*); this build wraps it. Not reconstructed.
    void _ReplaceObject(GameObjectId from, GameObjectId to, Symbol com);  // 0xF25C0
    // Exits the entity if it is entered, destroys its objects, the root
    // last, and destroys the entity.
    void _Destroy();  // 0xF65C0
    // Enters the entity in the game mode, or the edit mode when bit 0 of
    // the flags is set, and enters its objects unless bit 1 is set. Name
    // not in the reference map, which has _Enter().
    void Enter(unsigned int flags);  // 0xF4D10
    // Exits the entity's objects unless bit 0 of the flags is set. Name not
    // in the reference map.
    void Exit(unsigned int flags);  // 0xF50D0
    // Polls the objects of an active entity: the poll window in the game
    // mode, every object in the edit mode.
    void _Poll();  // 0xF5FC0
    // Post-polls the objects. Not reconstructed.
    void _PostPoll();  // 0xF6380
    // Queues the component's post-poll on the thread's poll dependencies.
    // Not reconstructed.
    void QueueForPostPoll(const Component* com);  // 0xF5F40
    // The number of components of the entity's objects, with those of the
    // entities their instance components hold when `instances` is set.
    // Name not in the reference map. Not reconstructed.
    unsigned long GetNumComs(bool instances) const;  // 0x114E90
    // Enter and poll at once when the thread's poll context asks for it
    // (see ThreadPollContext). Names not in the reference map. Not
    // reconstructed.
    void _EnterImmediately();  // 0xF53A0
    void _PollImmediately();   // 0xF5580
    // When the entity is entered in the game mode and the thread enters
    // at once, rebuilds the poll order, leaves the post-poll queue, calls
    // every component's _PreLoadResources and resets the poll job. Name not
    // in the reference map.
    void _PreLoadResources();  // 0xF5470

    // The object with the id, which must exist. Inlined into
    // RndDefaults::_SyncEnabledLights at 0x6BFAA4; the map only has
    // GetObject(Symbol) const.
    GameObject* GetObject(GameObjectId id) const {
        return mLayers[id.Layer()].mObjects[id.Index()];
    }

    // The entity's root object, the first object of its first layer.
    // Inlined into RndDefaults::_LoadLighting at 0x6BEFDE. Name not in the
    // reference map.
    GameObject* GetRoot() const {
        return mLayers[0].mObjects[0];
    }

    // Field names are not in the reference map.
    // The layers, the map's PropArray<Entity::Layer>.
    PropArray<Layer> mLayers;
    // The objects in poll order and the objects that post-poll (vtable
    // 0x18E6A28), rebuilt by the map's _UpdatePollOrder() and
    // _UpdatePostPollOrder() (0xF3CB0).
    PropArray<GameObjectId> mPollOrder;
    PropArray<GameObjectId> mPostPollOrder;
    // The resource that owns the entity.
    EntityResource* mResource;
    // The range of mPollOrder that _Poll polls in the game mode. Their
    // writers are not identified.
    unsigned short mPollWindowStart;
    unsigned short mPollWindowEnd;
    // State flags. Bit 0 marks an entity built in place, which _Destroy
    // destructs without deleting (0x200 then keeps its storage); bits 1-2
    // hold the mode it is entered in (1 the game mode, 2 the edit mode),
    // 0x10 is set while it is entered, 0x20 once its resources are loaded,
    // 0x100 while it is active, 0x400 marks changed components, 0x800 comes
    // from the resource (EntityResource slot 30), 0x1000 marks an entity
    // entered at once and 0x10000 is set while the root object's components
    // load their resources. The constructor sets 0x40 (polling enabled,
    // SetPollEnabled), 0x80 (cleared by 0xF57E0 when an object is not
    // lightweight) and 0x100. Enter sets 0x8 while it runs and 0x8000 when
    // the root's EntityHeaderCom drives the parent; 0x4000, which SetParent
    // sets, is cleared after a poll and 0x2000 keeps the entity from
    // polling.
    unsigned int mFlags;
    // The object in the parent entity that instances this one, or null.
    GameObject* mParentObject;
    // The class id of the parent object's component that instances the
    // entity (InstanceCom passes its GetId), set with mParentObject by
    // SetParent; the constructor leaves it empty. Its readers are not
    // identified.
    Symbol mParentComId;
};

// Opens a frame of the object-id fixup table while an entity loads
// (EntityResource::_LoadEntity). The map has it in entity/Entity.o; this
// build places it at 0x115010. Not reconstructed.
class PushFixupEntity {
public:
    PushFixupEntity();   // 0x115010
    ~PushFixupEntity();  // 0x115040
};

// Destroys the object with the id when it exists and clears the id. The
// map has it in entity/EntityUtl.o with the signature
// SafeDestroyObject(EntityPtr const&, GameObjectId&). Not reconstructed.
void SafeDestroyObject(Entity* entity, GameObjectId& id);  // 0x114FB0

static_assert(sizeof(Entity::Layer) == 48);
static_assert(offsetof(Entity::Layer, mNextSerial) == 40);
static_assert(offsetof(Entity, mEventSinks) == 8);
static_assert(offsetof(Entity, mPollNode) == 32);
static_assert(offsetof(Entity, mLayers) == 168);
static_assert(offsetof(Entity, mResource) == 288);
static_assert(offsetof(Entity, mPollWindowStart) == 296);
static_assert(offsetof(Entity, mFlags) == 300);
static_assert(offsetof(Entity, mParentObject) == 304);
static_assert(sizeof(Entity) == 320);

// Whether the entity's resource holds an inlined resource with the path
// (entity/Utl.o). The map's IsResourcePathInlined(ResourcePath, EntityPtr
// const&). Not reconstructed.
bool IsResourcePathInlined(ResourcePath path, Entity* entity);  // 0x114CC0
// A name for a new object of the entity: the name, made lowercase with
// underscores between words when `lowercase` is set, with a number added or
// raised until no object has it (entity/Utl.o). The map's
// GenerateUniqueObjectName(Symbol, EntityPtr const&, bool). Not
// reconstructed.
Symbol GenerateUniqueObjectName(Symbol name, Entity* entity, bool lowercase);  // 0x113A50
