#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "stategraph/nodes/StateNode.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The node that runs its child nodes as a group (stategraph/StateNodeGroup.o,
// 0x71AC90-0x71E667; class id "StateNodeGroup"): all at once, one after
// another (optionally shuffled and repeated), or one picked at random. The
// children are other nodes of the same folder that no other group runs;
// each knows the group through StateNode::SetGroup. The instances are
// SimultaneousInstance, SequenceInstance and PickOneInstance, declared in
// the source. The vtable at 0x193BEE8 has 50 slots. The object is 232
// bytes.
class StateNodeGroup : public StateNode {
public:
    // How the children run ("mode"). The labels are the allowed values';
    // the enumerator names are not in the reference map.
    enum Mode : int {
        kSimultaneous = 0,
        kSequence = 1,
        kPickOne = 2,
    };

    StateNodeGroup();  // 0x71AC90
    // Copies the group for an imprint; the resolved children start empty.
    // The map has it out of line; this build inlines it into _Imprint
    // (0x71C380).
    StateNodeGroup(const StateNodeGroup& other)
        : StateNode(other),
          mChildNodes(other.mChildNodes),
          mMode(other.mMode),
          mShuffle(other.mShuffle),
          mRepeat(other.mRepeat),
          mChildren() {}
    // Slot 0 at 0x71B750 jumps to the body at 0x6FFE20, which
    // stategraph/StateGraph.o emits; slot 1 is 0x71C2E0.
    ~StateNodeGroup() override;

    Symbol GetId() const override;          // slot 4: 0x71C300
    Symbol GetClassName() const override;   // slot 5: 0x71C310
    int CurrentRev() const override;        // slot 7: 0x71C320
    bool IsA(Symbol type) const override;   // slot 8: 0x71C340
    Component* AsComponent() override;      // slot 9: 0x71C370
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x71C380
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x71C5B0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x71C5C0
    // Slot 41: resolves the children and makes this their group.
    void _Fixup() override;  // 0x71B760
    // Slot 42: "<n> nodes", counting the set children.
    const char* GetEditorText() const override;  // 0x71BD10
    // Slot 43: the children and their own child nodes.
    void GetChildNodes(eastl::vector<GameObjectId>& children) const override;  // 0x71BEB0
    // Slot 45: the instance for the mode.
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // 0x71C030
    bool CanBlend() const override;  // slot 46: 0x71C5D0, true
    // Slots 47-48: whether any child does.
    bool UsesColorBlending() const override;     // 0x71B8C0
    bool UsesRotationBlending() const override;  // 0x71B910
    bool _HasActivity() const override;  // slot 49: 0x71C5E0, false

    // Appends the node unless it is a child already. No caller in this
    // build.
    void AddChild(GameObjectId child);  // 0x71B960
    // Removes the node from the children. No caller in this build.
    void RemoveChild(GameObjectId child);  // 0x71BB20
    // Replaces the child at the index. No caller in this build.
    void SetChild(unsigned long index, GameObjectId child);  // 0x71BCA0
    // Forgets the resolved child, keeping its id. No caller in this build.
    // Name not in the reference map; the map's GetChild(unsigned int) const
    // has no match in this build.
    void ForgetChildNode(const StateNode* child);  // 0x71BFF0

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F9B30.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FFDD0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x71AD50

    static Symbol sId;  // 0x1AB3E00, "StateNodeGroup"
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB3E08
    static PropRegistry sPropRegistry;  // 0x1AB3E10
    static ComMetaData sMetaData;       // 0x1AB3EB0

    // Field names are not in the reference map; they follow the
    // properties.
    // "child_nodes": the objects of the children.
    PropArray<GameObjectId> mChildNodes;
    // "mode" (Mode).
    int mMode;
    // "shuffle" and "repeat", for a sequence.
    bool mShuffle;
    bool mRepeat;
    // The nodes of mChildNodes, resolved by _Fixup; null for an unset or
    // missing child.
    eastl::vector<StateNode*> mChildren;
};

static_assert(offsetof(StateNodeGroup, mChildNodes) == 152);
static_assert(offsetof(StateNodeGroup, mMode) == 192);
static_assert(offsetof(StateNodeGroup, mShuffle) == 196);
static_assert(offsetof(StateNodeGroup, mRepeat) == 197);
static_assert(offsetof(StateNodeGroup, mChildren) == 200);
static_assert(sizeof(StateNodeGroup) == 232);
