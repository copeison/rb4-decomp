#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class BinStream;
class EntityResource;

// A set of game objects grouped into layers (entity/Entity.o). Only the
// layer table, the owning resource and the object API the reconstructed
// code uses are modelled; the 320-byte object is built by the constructor
// at 0xEBB70.
class Entity {
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
    };

    // Builds an entity with the given number of empty layers. The map's
    // signature is Entity(); this build passes the layer count. Not
    // reconstructed: it builds the MsgSource and PollDepBase bases, which
    // are not modelled.
    explicit Entity(unsigned long numLayers);  // 0xEBB70

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
    // The first object of any layer. The map's signature is
    // BeginObject() const; this build forwards to NextObject.
    GameObject* BeginObject() const;  // 0xEF180
    // The object after the given one that holds a component with the class
    // or base-class symbol; the empty symbol matches any object. The map's
    // signature is NextObject(GameObjectId, Symbol) const; this build takes
    // the object.
    GameObject* NextObject(const GameObject* object, Symbol com) const;  // 0xEF1A0
    // A description for error messages: the resource's id and path, and
    // the resource it is inlined in.
    const char* MakeErrorName() const;  // 0xF0DD0
    // The entity whose instance object holds this one, or null.
    Entity* GetParent() const;  // 0xF0780
    // Whether the entity resource has the layer; the main layer always
    // exists. Forwards to the resource. The map's LayerExists(unsigned
    // long) const.
    bool LayerExists(unsigned long layer) const;  // 0xF25B0
    // The layer's file. Forwards to the resource. The map's
    // GetLayerPath(unsigned long) const.
    ResourcePath GetLayerPath(unsigned long layer) const;  // 0xF3050

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
    // Exits the entity if it is entered and destroys its objects and the
    // entity. Not reconstructed.
    void _Destroy();  // 0xF65C0
    // Enters and exits the entity's objects with the given flags. Names
    // not in the reference map, which has _Enter(). Not reconstructed.
    void Enter(unsigned int flags);  // 0xF4D10
    void Exit(unsigned int flags);   // 0xF50D0
    // Polls the entity's objects. Not reconstructed.
    void _Poll();  // 0xF5FC0
    // Enter and poll at once when the thread's poll context asks for it
    // (see ThreadPollContext). Names not in the reference map. Not
    // reconstructed.
    void _EnterImmediately();  // 0xF53A0
    void _PollImmediately();   // 0xF5580

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

    // Field names are not in the reference map. The constructor (0xEBB70)
    // installs the vtables of the MsgSource base at 0 and the PollDepBase
    // base at 24; the map lists Entity::s_MsgSource_vtable and
    // s_PollDepBase_vtable.
    unsigned char mMsgSourceBase[24];
    unsigned char mPollDepBase[144];
    // The layers, the map's PropArray<Entity::Layer>.
    PropArray<Layer> mLayers;
    // The objects in poll order and the objects that post-poll (vtable
    // 0x18E6A28), rebuilt by the map's _UpdatePollOrder() and
    // _UpdatePostPollOrder() (0xF3CB0).
    PropArray<GameObjectId> mPollOrder;
    PropArray<GameObjectId> mPostPollOrder;
    // The resource that owns the entity.
    EntityResource* mResource;
    // Zeroed by the constructor; not identified.
    unsigned char mPadding[4];
    // State flags. Bit 0 marks an entity built in place, which _Destroy
    // destructs without deleting (0x200 then keeps its storage); bits 1-2
    // hold the mode it is entered in (1 the game mode, 2 the edit mode),
    // 0x10 is set while it is entered, 0x20 once its resources are loaded,
    // 0x100 while it is active, 0x400 marks changed components, 0x800 comes
    // from the resource (EntityResource slot 30), 0x1000 marks an entity
    // entered at once and 0x10000 is set while the root object's components
    // load their resources.
    unsigned int mFlags;
    // The object in the parent entity that instances this one, or null.
    GameObject* mParentObject;
    // An empty symbol set by the constructor; its readers are not
    // identified.
    Symbol mTag;
};

static_assert(sizeof(Entity::Layer) == 48);
static_assert(offsetof(Entity::Layer, mNextSerial) == 40);
static_assert(offsetof(Entity, mLayers) == 168);
static_assert(offsetof(Entity, mResource) == 288);
static_assert(offsetof(Entity, mFlags) == 300);
static_assert(offsetof(Entity, mParentObject) == 304);
static_assert(sizeof(Entity) == 320);
