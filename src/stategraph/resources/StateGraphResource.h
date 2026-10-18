#pragma once

#include <cstddef>

#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "entity/resources/ResourceMetaData.h"
#include "stategraph/conditions/StateCondition.h"
#include "utl/containers/Set.h"
#include "utl/text/Symbol.h"

class BinStream;
class Entity;
class StateBase;
class StateNode;
class StateNodeRoot;
class StateTransition;

// A state graph: an entity resource (extension "state") whose objects are
// the graph's nodes, transitions and conditions, with a StateNodeRoot on
// the root object (stategraph/StateGraphResource.o, 0x706C40-0x70840B).
// Graphs are inlined into the entity resources that run them, whose
// StateGraphDriverCom plays them. The vtable at 0x193A6A0 has 33 slots.
// The object is 200 bytes.
class StateGraphResource : public EntityResource {
public:
    // The class symbol, "StateGraphResource", kept in a guarded static
    // (0x1AB2708) that is built on first use.
    static Symbol Id() {
        static Symbol sId;
        if (sId == Symbol()) {
            sId = Symbol("StateGraphResource");
        }
        return sId;
    }

    // Registers the class: its metadata and its factory. Inline in the
    // map's stategraph/StateGraph.o (StateGraphResource::Init()); emitted
    // there at 0x6F61C0.
    static void Init() {
        _Init(sMetaData);
        const Symbol id = Id();
        Resource::sFactory[id] = &_Create;
        sMetaData.Init(id, Symbol("EntityResource"), true);
    }
    // The factory. Emitted in stategraph/StateGraph.o at 0x6FED60.
    static Resource* _Create() {
        return new StateGraphResource;
    }
    // Names the type's extension ("state").
    static void _Init(ResourceMetaData& metaData);  // 0x706C40

    StateGraphResource();  // 0x706D40
    // Slots 10-11: 0x708350 and 0x708360.
    ~StateGraphResource() override {}

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x708270
    Symbol GetId() const override;                   // slot 1: 0x708280
    bool IsA(Symbol type) const override;            // slot 2: 0x708320
    // Slot 5: the entity resource, then the graph's revision, 2.
    void Save(BinStream& stream, bool cached) override;  // 0x706DB0
    // Slot 13: a new entity with a StateNodeRoot on its root.
    Entity* CreateEntity() override;  // 0x706D70
    // Slot 14: loads the entity and the graph's revision; a graph saved
    // before revision 1 has its root's StateNode replaced by a
    // StateNodeRoot that takes its properties.
    bool _LoadEntity(BinStream& stream, bool cached) override;  // 0x706E10
    // Slot 15: before revision 1, points every transition at the node that
    // lists it, and loads the transitions' blend curves.
    bool _PostLoad(BinStream& stream, bool cached) override;  // 0x7070C0

    // The root node.
    StateNodeRoot* GetRootNode() const;  // 0x707200
    bool IsRootNode(const StateNode* node) const;  // 0x707260
    // The root's "start_on_enter", which must exist. Names not in the
    // reference map.
    bool StartsOnEnter() const;              // 0x7072C0
    void SetStartsOnEnter(bool startOnEnter);  // 0x707310
    // Creates an object holding a new component of the class, named after
    // the class (GetBaseName).
    GameObject* CreateGraphObject(Symbol className, const char* prefix);  // 0x707380
    // Names a new graph object after its component's class and reenters
    // its components. The map's signature takes an ObjPtr&.
    void _PostCreateObject(StateBase* component, GameObject* object, const char* prefix);  // 0x7073F0
    // Creates an object holding a new condition of the type.
    GameObject* CreateGraphObject(StateCondition::Type type, const char* prefix);  // 0x707430
    // The object with the id, or null for the null id. The map's signature
    // returns an ObjPtr.
    GameObject* FindGraphObject(GameObjectId id) const;  // 0x707490
    // The base of a graph object's name: the class symbol after the prefix
    // and before the first underscore, or "No-Op" for the base StateNode.
    static Symbol GetBaseName(Symbol className, const char* prefix);  // 0x7074D0
    // Destroys the object with the id.
    void DestroyGraphObject(GameObjectId id);  // 0x7075F0
    // The transition whose condition, or one of its descendants, is the
    // condition with the id. Name not in the reference map.
    StateTransition* FindTransitionWithCondition(GameObjectId condition) const;  // 0x707630
    // The blend tree node that is the node with the id or holds it as a
    // child. Name not in the reference map.
    StateNode* FindParentBlendTree(GameObjectId node) const;  // 0x7077C0
    // The entity the graph is the prototype of: the entity of the entity
    // resource the graph is inlined into, or null.
    Entity* GetContextEntityPrototype() const;  // 0x707920
    // Calls StateBase::Fixup on every graph component. Name not in the
    // reference map.
    void FixupAll();  // 0x707940
    // Adds the ids of the transitions and conditions no node reaches;
    // true when there are any. Name not in the reference map.
    bool GetUnusedObjects(eastl::set<unsigned int>& unused) const;  // 0x707A00

    // The class's metadata.
    static ResourceMetaData sMetaData;  // 0x1AB2C60

    // The graph's revision, read after the entity; 0 for graphs saved
    // before it was written. It sits in EntityResource's tail padding.
    // Name not in the reference map.
    int mRev;
};

static_assert(offsetof(StateGraphResource, mRev) == 196);
static_assert(sizeof(StateGraphResource) == 200);
