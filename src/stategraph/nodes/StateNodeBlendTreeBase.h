#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/blendtrees/BlendTreeBaseCom.h"
#include "stategraph/nodes/StateNode.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The base of the nodes that run a blend tree
// (stategraph/StateNodeBlendTreeBase.o, 0x717900-0x718EFA, after the map's
// build; class id "StateNodeBlendTreeBase"). The node sits on the object of
// its tree's BlendTreeBaseCom, plays the tree while it is active, and is the
// StateNodeTiming transitions synchronize to, after StateNode at offset 152.
// The object also holds StateNodeClipBlendTree's and
// StateNodeClipJukebox's registries; the three share one static
// initializer. The base may not be created in the editor. The vtable at
// 0x193B910 has 56 slots; the StateNodeTiming one at 0x193BAE0 holds the
// thunks. The object is 160 bytes. The metadata names the class
// "StateNodeBlendTree" (InitSuperclass), as does the instance's pool label;
// that may be the original name.
class StateNodeBlendTreeBase : public StateNode, public StateNodeTiming {
public:
    // The node's run-time state: the tree's instance, played at the node's
    // weight in the graph. The vtable at 0x193B888 has 15 slots. The object
    // is 48 bytes, from the small-block pool. The pool label is
    // "StateNodeBlendTree::Instance".
    class Instance : public StateNodeInstance {
    public:
        static void* operator new(unsigned long size) {
            return PoolAlloc(size, "StateNodeBlendTree::Instance");
        }
        static void operator delete(void* allocation) {
            PoolFree(sizeof(Instance), allocation);
        }

        // Called by _CreateInstance (0x6FF8B0).
        Instance(const StateNodeBlendTreeBase& node, StateGraphInstance& graph);  // 0x717FD0
        // Slots 0-1: 0x718070, 0x7180B0.
        ~Instance() override;

        // Slot 3 at 0x718D50: the weight the tree plays at.
        void _SetWeight(float weight) override {
            mWeight = weight;
        }
        // Slot 5: plays the tree; the node never finishes by itself.
        bool _Poll() override;  // 0x718100
        // Slot 6: stops the tree and drops its instance.
        void _Exit() override;  // 0x718120
        // Slot 11: the tree's instance when the tree is on the object or
        // holds a subtree that is. The binary returns the
        // BlendTreeBaseCom::Instance in the StateNodeInstance slot.
        StateNodeInstance* _FindInstance(const GameObject* object) override;  // 0x718160
        // Slots 12-13: the tree's timing position.
        float _GetTimingPosition() override;               // 0x7181A0
        void _SetTimingPosition(float position) override;  // 0x7181B0

        // Field names are not in the reference map.
        BlendTreeBaseCom::Instance* mTreeInstance;
        float mWeight;
    };

    StateNodeBlendTreeBase() {}
    // Copies the node for an imprint. Inlined into _Imprint (0x718B80).
    StateNodeBlendTreeBase(const StateNodeBlendTreeBase& other) : StateNode(other), StateNodeTiming(other) {}
    // Slots 0-1: 0x717BC0, 0x718A60.
    ~StateNodeBlendTreeBase() override;

    Symbol GetId() const override;          // slot 4: 0x718B00
    Symbol GetClassName() const override;   // slot 5: 0x718B10
    int CurrentRev() const override;        // slot 7: 0x718B20
    bool IsA(Symbol type) const override;   // slot 8: 0x718B40
    Component* AsComponent() override;      // slot 9: 0x718B70
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x718B80
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x718D30
    ComMetaData& _GetMetaData() override;       // slot 23: 0x718D40
    // Slot 43: the objects of the tree's subtrees, at every depth.
    void GetChildNodes(eastl::vector<GameObjectId>& children) const override;  // 0x717C50
    // Slot 44 at 0x6FF8A0: the node's own timing.
    const StateNodeTiming* GetTiming() const override {
        return this;
    }
    // Slot 45 at 0x6FF8B0.
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override {
        return new Instance(*this, graph);
    }
    // Slot 46 at 0x6FF8F0: the tree blends.
    bool CanBlend() const override {
        return true;
    }

    // Slots 50-55 and their StateNodeTiming thunks (0x717CE0, 0x717D00,
    // 0x717D80, 0x717E00, 0x717E80, 0x717F60), through the tree.
    bool IsValid() const override;  // 0x717CD0
    // The object's name. The binary jumps to StateBase::GetName, whose
    // symbol is the name's characters.
    const char* GetName() const override;  // 0x717CF0
    float GetScale(int units, float rate) const override;  // 0x717D10
    // A tree has no length of its own: zero, and false.
    bool GetLength(float& length) const override;  // 0x717DF0
    int GetUnits() const override;  // 0x717E10
    void Refresh() const override;  // 0x717EF0

    // The tree on the node's object. Inlined into its users. Name not in
    // the reference map.
    BlendTreeBaseCom* GetBlendTree() const {
        return mObject->GetBaseCom<BlendTreeBaseCom>();
    }

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F84A0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FF410.
    static Component* _Create();
    // Only the subclasses are creatable nodes.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x717900
    // Makes StateNodeBlendTreeBase the superclass of a subclass's metadata,
    // as StateNode::_InitAsSuperclass does. Name not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x718610

    static Symbol sId;  // 0x1AB3210, "StateNodeBlendTreeBase"
    // Also "StateNodeBlendTreeBase". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB3218
    static PropRegistry sPropRegistry;  // 0x1AB3220
    static ComMetaData sMetaData;       // 0x1AB32C0
};

static_assert(offsetof(StateNodeBlendTreeBase::Instance, mTreeInstance) == 32);
static_assert(offsetof(StateNodeBlendTreeBase::Instance, mWeight) == 40);
static_assert(sizeof(StateNodeBlendTreeBase::Instance) == 48);
static_assert(sizeof(StateNodeBlendTreeBase) == 160);
