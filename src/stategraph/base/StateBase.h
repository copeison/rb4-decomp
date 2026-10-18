#pragma once

#include <cstddef>
#include <functional>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropRegistry.h"
#include "utl/text/MakeString.h"
#include "utl/text/Symbol.h"

class Entity;
class StateGraphResource;
struct TagArgs;

// The base of every state graph component (stategraph/StateBase.o,
// 0x807E80-0x808AD5; class id "StateBase"): the nodes, transitions and
// conditions, each on its own object of a StateGraphResource's entity. It
// adds a comment, the object's name and id as properties, and slot 41,
// which resolves the object ids the component refers to. The vtable at
// 0x1950668 has 42 slots. The object is 32 bytes.
class StateBase : public Component {
public:
    StateBase() : mComment() {}
    // Copies the comment for an imprint; the component starts afresh.
    // Inlined into the subclasses' _Imprint, for example at 0x808670.
    StateBase(const StateBase& other) : Component(), mComment(other.mComment) {}
    // Slots 0-1: 0x808210 (Component's destructor) and 0x8085D0.
    ~StateBase() override {}

    Symbol GetId() const override;          // slot 4: 0x8085F0
    Symbol GetClassName() const override;   // slot 5: 0x808600
    int CurrentRev() const override;        // slot 7: 0x808610
    bool IsA(Symbol type) const override;   // slot 8: 0x808630
    Component* AsComponent() override;      // slot 9: 0x808660
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x808670
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x8087A0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x8087B0
    // Slot 29: resolves the object ids once the resources are loaded.
    bool _OnResourcesLoaded() override;  // 0x808220
    // Slot 41 at 0x8087C0: resolves the object ids the component refers to
    // into pointers; nothing here. The map has StateBase::_Fixup(bool); this
    // build drops the flag. The evidence for the name is weak.
    virtual void _Fixup() {}

    // Calls _Fixup; StateGraphResource::FixupAll (0x707940) calls it on
    // every component of the graph. Name not in the reference map.
    void Fixup();  // 0x808240
    // The entity the graph is the prototype of: the graph resource's
    // inlining entity resource's entity, or null.
    Entity* _GetContextEntityPrototype() const;  // 0x808250
    // The StateGraphResource that owns the component's entity, or null.
    // Name not in the reference map.
    StateGraphResource* _GetStateGraphResource() const;  // 0x808330
    // The tag function the driven properties of the graph objects take
    // their context entity from (sContextEntityTagFunc).
    static std::function<Entity*(const TagArgs&)>& _GetContextEntityTagFunc();  // 0x808410
    // The object's name.
    Symbol GetName() const;  // 0x808420
    // Tells the entity's sinks that the component's text in the editor
    // changed: ("appearance_updated" <object id>).
    void _SendDisplayTextUpdated() const;  // 0x808430

    // The component of class T on the graph object with the id, by class
    // id (GetCom) or by base class (GetBaseCom); null for the null id. When
    // `fail` is set, a missing component is reported, which this build
    // compiles out. Each object instantiates the ones it uses: StateNode.o
    // has _GetObjectCom<StateNodeFolder> at 0x70A9A0 and
    // _GetObjectBaseCom<StateTransition> at 0x70A130, StateTransition.o
    // _GetObjectBaseCom<StateCondition> at 0x7FB350. Names not in the
    // reference map.
    template <class T>
    T* _GetObjectCom(GameObjectId id, bool fail) const;
    template <class T>
    T* _GetObjectBaseCom(GameObjectId id, bool fail) const;

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6F6730.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FEE00.
    static Component* _Create();
    // Registers "name", "comment" and "object_id" and the class
    // description, and sets sContextEntityTagFunc the first time.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x807EB0
    // Makes StateBase the superclass of a subclass's metadata, runs _Init on
    // it in the "metadata" heap, and combines the editor restrictions _Init
    // sets with those the metadata had. Emitted in stategraph/StateNode.o.
    // Name not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x7092A0

    static Symbol sId;  // 0x1ABEDA0, "StateBase"
    // Also "StateBase". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABEDA8
    static PropRegistry sPropRegistry;  // 0x1ABEDB0
    static ComMetaData sMetaData;       // 0x1ABEE50
    static std::function<Entity*(const TagArgs&)> sContextEntityTagFunc;  // 0x1ABF000

    // "comment", shown in the node's tool tip. Name not in the reference
    // map.
    Symbol mComment;
};

// Reports a graph object without the component a link names. The report
// is compiled out; its arguments remain. Inlined into the lookups. Name not
// in the reference map.
inline void ReportMissingGraphCom(const StateBase& base, const GameObject* object) {
    if (object != nullptr) {
        FormatString message("'%s' does not have the specified component!");
        message << object->GetSafeName();
        static_cast<void>(message.Str());
    }
    static_cast<void>(base.MakeErrorName());
}

template <class T>
T* StateBase::_GetObjectCom(GameObjectId id, bool fail) const {
    if (id.mId == 0xFFFFFFFFU) {
        return nullptr;
    }
    const GameObject* const object = mObject->mEntity->GetObject(id);
    T* const com = object != nullptr ? object->GetCom<T>() : nullptr;
    if (com == nullptr && fail) {
        ReportMissingGraphCom(*this, object);
    }
    return com;
}

template <class T>
T* StateBase::_GetObjectBaseCom(GameObjectId id, bool fail) const {
    if (id.mId == 0xFFFFFFFFU) {
        return nullptr;
    }
    const GameObject* const object = mObject->mEntity->GetObject(id);
    T* const com = object != nullptr ? object->GetBaseCom<T>() : nullptr;
    if (com == nullptr && fail) {
        ReportMissingGraphCom(*this, object);
    }
    return com;
}

static_assert(offsetof(StateBase, mComment) == 24);
static_assert(sizeof(StateBase) == 32);
