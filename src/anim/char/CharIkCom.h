#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropRegistry.h"
#include "utl/containers/LinkedList.h"
#include "utl/text/Symbol.h"

class CharSkeleton;
class CharSkeletonPose;

// The base of the inverse-kinematics components (anim/CharIk.o,
// 0x2F1360-0x2F157F; class id "CharIk"): "Base class for all CharIk
// components". A CharIkDriverCom links the IK components it runs through
// mIkNode and applies them to a character's pose in its order. The class is
// registered but not created itself. The vtable at 0x18F1958 has 43 slots;
// slots 41 and 42 are the class's own. The object is 40 bytes. The virtual
// methods are inline in the map's header, so the binary emits them in
// anim/AnimInit.o (0x2B6020-0x2B6274), where the registration creates a
// prototype; this source defines them in CharIk.cpp.
class CharIkCom : public Component {
public:
    // Reaches the link that keeps a component in its driver's list. Name not
    // in the reference map.
    class IkNode {
    public:
        static LinkedList::Node& ToNode(CharIkCom& ik) {
            return ik.mIkNode;
        }
        static CharIkCom* FromNode(LinkedList::Node* node) {
            return reinterpret_cast<CharIkCom*>(reinterpret_cast<char*>(node) - offsetof(CharIkCom, mIkNode));
        }
    };

    // Inlined into the factory (0x2B5FA0) and the subclasses' constructors.
    CharIkCom() : mIkNode() {}
    // Copies the component for an imprint, unlinked. Inlined into _Imprint
    // (0x2B6110) and the subclasses' copies.
    CharIkCom(const CharIkCom& other) : Component(other), mIkNode() {}
    // Slots 0-1: 0x2B6020, 0x2B6050. Unlinks the component.
    ~CharIkCom() override;

    Symbol GetId() const override;          // slot 4: 0x2B6090
    Symbol GetClassName() const override;   // slot 5: 0x2B60A0
    int CurrentRev() const override;        // slot 7: 0x2B60B0
    bool IsA(Symbol type) const override;   // slot 8: 0x2B60D0
    Component* AsComponent() override;      // slot 9: 0x2B6100
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2B6110
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2B6240
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2B6250

    // Slot 41: moves the pose's bones; nothing here.
    virtual void Apply(CharSkeletonPose& pose, const CharSkeleton* skeleton);  // 0x2B6260
    // Slot 42: 0 here, and in every subclass of this build; nothing calls
    // it.
    virtual float GetPriority() const;  // 0x2B6270

    // Registers the class (InitComponentClass) as one that is not created.
    // Inline in the map; this build emits it in anim/AnimInit.o at
    // 0x2AEBE0.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B5FA0; the
    // placement factory beside it is at 0x2B5FE0.
    static Component* _Create();
    // Describes the class and limits it to CharResource entities.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x2F1360
    // Makes the class the superclass of a subclass's metadata and runs
    // _Init on the subclass's registry in the "metadata" heap, keeping the
    // stricter editor restrictions. The subclasses' _Init call it; the
    // binary emits it once, in anim/CharIkBoneInterpCom.o, and other
    // modules' IK components call it too (0x3C39BA, 0xFF9C98, 0x10544A8,
    // 0x106FBC3). Name not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x2F3C20

    // "CharIk", constructed by the static initializer at 0x2F14C0, which
    // first sets a shared header's null object id to -1 (0x19F8E20).
    static Symbol sId;  // 0x19F8E28
    // Also "CharIk". Name not in the reference map.
    static Symbol sClassName;           // 0x19F8E30
    static PropRegistry sPropRegistry;  // 0x19F8E40
    static ComMetaData sMetaData;       // 0x19F8EE0

    // The link in the driver's list (CharIkDriverCom::mIks). Name not in
    // the reference map.
    LinkedList::Node mIkNode;
};

static_assert(offsetof(CharIkCom, mIkNode) == 24);
static_assert(sizeof(CharIkCom) == 40);
