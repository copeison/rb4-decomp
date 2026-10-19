#pragma once

#include <cstddef>

#include "anim/char/CharIkCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "utl/containers/LinkedList.h"
#include "utl/text/Symbol.h"

class CharSkeleton;
class CharSkeletonPose;

// The component that runs a character's IK components
// (anim/CharIkDriverCom.o, 0x2F5120-0x2F66FF; class id "CharIkDriver"):
// "Put on the root object to allow char anim to include Inverse Kinematics
// (ik)." Its "ik" array names CharIkCom components by object and class;
// when its resources load it links them, in the array's order, and Apply
// runs them. It is itself a CharIkCom, and only the root object may hold
// it. The vtable at 0x18F6C68 has 43 slots. The object is 96 bytes. Field
// names follow the registry.
class CharIkDriverCom : public CharIkCom {
public:
    // One IK component to run, an element of "ik" (16 bytes).
    struct IkRef {
        // "object": "Object id of the game object that the component is
        // on", one holding a CharIkCom.
        GameObjectId mObject = {0xFFFFFFFFU};
        // "component": "Component name of the world ik component", one of
        // the object's CharIkCom classes.
        Symbol mComponent;
    };

    // Inlined into the factory (0x2B6280).
    CharIkDriverCom() : mIk(), mIks() {}
    // Copies the component for an imprint, with no components linked.
    // Inlined into _Imprint (0x2F5CC0).
    CharIkDriverCom(const CharIkDriverCom& other) : CharIkCom(other), mIk(other.mIk), mIks() {}
    // Slots 0-1: 0x2F5910, 0x2F5B90.
    ~CharIkDriverCom() override;

    Symbol GetId() const override;          // slot 4: 0x2F5C40
    Symbol GetClassName() const override;   // slot 5: 0x2F5C50
    int CurrentRev() const override;        // slot 7: 0x2F5C60
    bool IsA(Symbol type) const override;   // slot 8: 0x2F5C80
    Component* AsComponent() override;      // slot 9: 0x2F5CB0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2F5CC0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2F5E30
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2F5E40
    // Slot 29: unlinks the components and links again, in order, each one
    // "ik" names that exists and is a CharIkCom. The map's
    // _LoadResources(ObjPtr const&) has this body.
    bool _OnResourcesLoaded() override;  // 0x2F5A10
    // Slot 41: applies the linked components in order.
    void Apply(CharSkeletonPose& pose, const CharSkeleton* skeleton) override;  // 0x2F59B0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in anim/AnimInit.o at 0x2AEFC0.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B6280; the
    // placement factory beside it is at 0x2B6300.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x2F5120

    // "CharIkDriver", constructed by the static initializer at 0x2F6640,
    // which first sets the object's null object id to -1 (0x19F9358).
    static Symbol sId;  // 0x19F9360
    // Also "CharIkDriver". Name not in the reference map.
    static Symbol sClassName;           // 0x19F9368
    static PropRegistry sPropRegistry;  // 0x19F9370
    static ComMetaData sMetaData;       // 0x19F9410

    // "ik".
    PropArray<IkRef> mIk;
    // The linked components, in the order of "ik". Name not in the
    // reference map.
    LinkedList::List<CharIkCom, CharIkCom::IkNode> mIks;
};

static_assert(offsetof(CharIkDriverCom::IkRef, mComponent) == 8);
static_assert(sizeof(CharIkDriverCom::IkRef) == 16);
static_assert(offsetof(CharIkDriverCom, mIk) == 40);
static_assert(offsetof(CharIkDriverCom, mIks) == 80);
static_assert(sizeof(CharIkDriverCom) == 96);
