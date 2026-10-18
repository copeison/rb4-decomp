#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "math/geometry/Rect.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/base/StateBase.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class Entity;
class StateGraphInstance;
class StateNodeFolder;
class StateNodeGroup;
class StateNodeInstance;
class StateTransition;

// The animation timing a node exposes through StateNode slot 44: the
// timing of a clip or blend tree that a transition can synchronize to. The
// blend trees keep one inside the node (vtable 0x193A078, six slots). Only
// the slots the state graph reads are declared. Name not in the reference
// map; the slot names follow their uses and the evidence is weak.
class StateNodeTiming {
public:
    // Slot 0: whether the timing can be synchronized to.
    virtual bool IsValid() const = 0;
    // Slot 1: the node's name.
    virtual Symbol GetName() const = 0;
    // Slot 2: the scale of the timing in the units at the rate.
    virtual float GetScale(int units, float rate) const = 0;
    // Slot 3: the length, through `length`.
    virtual bool GetLength(float& length) const = 0;
    // Slot 4: the units the timing counts in.
    virtual int GetUnits() const = 0;
};

// A state of a state graph (stategraph/StateNode.o, 0x708410-0x70AD77;
// class id "StateNode"). The base node does nothing itself (the editor's
// "No-Op"); its subclasses animate, set properties, play sounds or nest
// other nodes. A node lists the transitions that leave it and may sit in
// a folder; StateGraphInstance runs the nodes through their
// StateNodeInstance. The vtable at 0x193A7B8 has 50 slots. The object is
// 152 bytes.
class StateNode : public StateBase {
public:
    // When the node starts working ("active_at") and stops ("active_until"),
    // relative to the transitions into and out of it. The labels are the
    // allowed values'; the enumerator names are not in the reference map.
    enum ActiveAt : int {
        kActiveAtStartOfIncoming = 0,
        kActiveAtEndOfIncoming = 1,
    };
    enum ActiveUntil : int {
        kActiveUntilStartOfOutgoing = 0,
        kActiveUntilEndOfOutgoing = 1,
    };
    // The constructors' value until _PostCreate picks the default for the
    // node's blending. Name not in the reference map.
    static constexpr int kActivityUnset = 2;

    // The node's links, resolved by _Fixup and not copied into imprints.
    // Name and field names not in the reference map.
    struct RuntimeData {
        RuntimeData();  // 0x7084C0

        // The folder the node sits in, from mFolder.
        StateNodeFolder* mFolderNode;
        // The group that runs the node as a child (SetGroup).
        StateNodeGroup* mGroup;
        // The transitions of mTransitions.
        eastl::vector<StateTransition*> mTransitions;
    };

    StateNode();  // 0x708410
    // Copies the node for an imprint; the links start empty. Inlined into
    // every node's _Imprint, for example at 0x709DD0.
    StateNode(const StateNode& other)
        : StateBase(other),
          mActiveAt(other.mActiveAt),
          mActiveUntil(other.mActiveUntil),
          mRect(other.mRect),
          mTransitions(),
          mFolder(other.mFolder),
          mRuntime() {
        mTransitions._Copy(other.mTransitions);
    }
    ~StateNode() override;  // slots 0-1: 0x709350, 0x709CB0

    Symbol GetId() const override;          // slot 4: 0x709D50
    Symbol GetClassName() const override;   // slot 5: 0x709D60
    int CurrentRev() const override;        // slot 7: 0x709D70
    bool IsA(Symbol type) const override;   // slot 8: 0x709D90
    Component* AsComponent() override;      // slot 9: 0x709DC0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x709DD0
    // Slot 19: picks the activity defaults for the node's blending.
    void _PostCreate() override;  // 0x7093E0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x709F80
    ComMetaData& _GetMetaData() override;       // slot 23: 0x709F90
    // Slot 41: resolves the folder and the transitions.
    void _Fixup() override;  // 0x709440

    // Slot 42 at 0x6FF890: the text the editor shows on the node; empty
    // here.
    virtual const char* GetEditorText() const {
        return "";
    }
    // Slot 43 at 0x7002C0: appends the objects of the nodes the node runs
    // as children; nothing here. Name not in the reference map.
    virtual void GetChildNodes(eastl::vector<GameObjectId>& children) const {
        static_cast<void>(children);
    }
    // Slot 44: the animation timing a transition can synchronize to; none
    // here. Name not in the reference map.
    virtual const StateNodeTiming* GetTiming() const;  // 0x7097B0
    // Slot 45: a new instance of the node for the graph.
    virtual StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const;  // 0x709830
    // Slot 46: whether the node can blend with the nodes before and after
    // it; false here. Name not in the reference map.
    virtual bool CanBlend() const;  // 0x709FA0
    // Slots 47-48 at 0x6FF900 and 0x6FF910: false here; the folder and the
    // groups answer whether any child does. No node in this build answers
    // true and only two unreferenced StateTransition helpers read them, so
    // the names are guesses. Names not in the reference map.
    virtual bool IsExclusive() const {
        return false;
    }
    virtual bool IsBlocking() const {
        return false;
    }
    // Slot 49 at 0x6FF920: whether the node does work of its own, which
    // makes "active_at" and "active_until" meaningful; true here, false for
    // the root, the folders and the groups. Name not in the reference map.
    virtual bool _HasActivity() const {
        return true;
    }

    // The timing of slot 44. Name not in the reference map.
    const StateNodeTiming* GetTimingSource() const;  // 0x7097A0
    // Whether the node has a timing that can be synchronized to. Name not
    // in the reference map.
    bool HasValidTiming() const;  // 0x7097C0
    // Whether the activity properties apply: never for the base node.
    // Name not in the reference map.
    bool HasActivity() const;  // 0x7097E0
    // Resets the activity defaults as _PostCreate does. No caller in this
    // build. Name not in the reference map.
    void ResetActivity();  // 0x709410
    // Adds the transition's object unless it is listed already.
    void AddTransition(GameObjectId transition);  // 0x709520
    // Removes the transition's object; true when it had a transition.
    bool RemoveTransition(GameObjectId transition);  // 0x709680
    // Moves the node into the folder. Name not in the reference map.
    void SetFolder(GameObjectId folder);  // 0x709770
    // A new instance of the node for the graph (slot 45).
    StateNodeInstance* CreateInstance(StateGraphInstance& graph) const;  // 0x709820
    // Sets and clears the group that runs the node. Names not in the
    // reference map.
    void SetGroup(StateNodeGroup* group);  // 0x709880
    void ClearGroup();                     // 0x709890

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6F6AF0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x6FEE90.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x7084E0
    // Makes StateNode the superclass of a subclass's metadata, as
    // StateBase::_InitAsSuperclass does. Emitted outside the object. Name
    // not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x70E940

    static Symbol sId;  // 0x1AB2CC0, "StateNode"
    // Also "StateNode". Name not in the reference map.
    static Symbol sClassName;  // 0x1AB2CC8
    static PropRegistry sPropRegistry;  // 0x1AB2CD0
    static ComMetaData sMetaData;       // 0x1AB2D70

    // Field names are not in the reference map; they follow the
    // properties.
    // "active_at" (ActiveAt) and "active_until" (ActiveUntil).
    int mActiveAt;
    int mActiveUntil;
    // "rect": the node's rectangle in the editor.
    Hmx::Rect mRect;
    // "transitions": the objects of the transitions that leave the node.
    PropArray<GameObjectId> mTransitions;
    // "folder": the object of the folder the node sits in.
    GameObjectId mFolder;
    RuntimeData mRuntime;
};

static_assert(offsetof(StateNode::RuntimeData, mTransitions) == 16);
static_assert(sizeof(StateNode::RuntimeData) == 48);
static_assert(offsetof(StateNode, mActiveAt) == 32);
static_assert(offsetof(StateNode, mActiveUntil) == 36);
static_assert(offsetof(StateNode, mRect) == 40);
static_assert(offsetof(StateNode, mTransitions) == 56);
static_assert(offsetof(StateNode, mFolder) == 96);
static_assert(offsetof(StateNode, mRuntime) == 104);
static_assert(sizeof(StateNode) == 152);

// The run-time state of a node in a running graph (the map's
// StateNodeInstance in stategraph/StateNode.o). StateGraphInstance enters,
// polls, weighs and exits it; the subclasses do the node's work in the
// protected slots. The vtable at 0x193A958 has 15 slots. The object is 32
// bytes, from the small-block pool.
class StateNodeInstance {
public:
    POOL_OVERLOAD(StateNodeInstance)

    StateNodeInstance(const StateNode& node, StateGraphInstance& graph);  // 0x7098A0
    // Slots 0-1: 0x7098D0, 0x7098E0.
    virtual ~StateNodeInstance();

    // Slot 2 at 0x709FB0: starts the node's work; nothing here.
    virtual void _Enter() {}
    // Slot 3 at 0x709FC0: the node's share of the graph's blend; nothing
    // here. The animation nodes weigh their clips with it. Name not in the
    // reference map.
    virtual void _SetWeight(float weight) {
        static_cast<void>(weight);
    }
    // Slot 4 at 0x709FD0: how far the transition into or out of the node
    // has blended; nothing here. The property nodes interpolate with it.
    // Name not in the reference map.
    virtual void _SetBlend(float blend) {
        static_cast<void>(blend);
    }
    // Slot 5: does the node's work; false once it is finished. The map's
    // _Poll(float); this build drops the time.
    virtual bool _Poll();  // 0x709FE0
    // Slot 6 at 0x709FF0: stops the node's work; nothing here.
    virtual void _Exit() {}
    // Slots 7-8: whether the instance wants to continue a blend partner,
    // and takes the earlier instance as its partner when it is one. False
    // here. Names not in the reference map.
    virtual bool _WantsBlendPartner() const;                // 0x70A000
    virtual bool _TakeBlendPartner(StateNodeInstance* previous);  // 0x70A010
    // Slots 9-10: the instance among this one's children that blends with
    // the other, and whether this one does. None here.
    virtual StateNodeInstance* _FindBlendPartner(const StateNodeInstance& other);  // 0x70A020
    virtual bool _IsBlendPartner(const StateNodeInstance& other) const;           // 0x70A030
    // Slot 11: the instance among this one's children that runs the node
    // on the object. None here. Name not in the reference map.
    virtual StateNodeInstance* _FindInstance(const GameObject* object);  // 0x70A040
    // Slots 12-13: the position on the node's timing and setting it. The
    // base reports that the node has no timing; the report is compiled
    // out. Names not in the reference map.
    virtual float _GetTimingPosition();               // 0x709C10
    virtual void _SetTimingPosition(float position);  // 0x709C40
    // Slot 14: whether the instance depends on the object, besides its
    // node's own. False here. Name not in the reference map.
    virtual bool _UsesObject(GameObjectId object) const;  // 0x709CA0

    // Enters the node unless it activates at the end of the transition
    // into it and other nodes still run.
    void Enter();  // 0x7098F0
    // Enters the node at once. No caller in this build. The map has
    // _UpdateEntered(); the evidence for the match is weak.
    void _UpdateEntered();  // 0x709930
    // Passes the node's weight and blend to slots 3 and 4 while it is
    // entered. Name not in the reference map.
    void SetWeights(float weight, float blend);  // 0x709950
    // Enters the node once it may, polls it until it finishes, and exits a
    // node that deactivates at the start of the transition out of it. True
    // while the node works. The map's Poll(float); this build drops the
    // time.
    bool Poll();  // 0x7099A0
    // Exits an entered node once.
    void Exit();  // 0x709A40
    // Slot 7 and slot 8. Names not in the reference map.
    bool WantsBlendPartner() const;                // 0x709A70
    bool TakeBlendPartner(StateNodeInstance* previous);  // 0x709A80
    // This instance when it blends with the other, or the one among its
    // children that does. Name not in the reference map.
    StateNodeInstance* FindBlendPartner(const StateNodeInstance& other);  // 0x709A90
    bool IsBlendPartner(const StateNodeInstance& other) const;            // 0x709B00
    // Slot 11. Name not in the reference map.
    StateNodeInstance* FindInstance(const GameObject* object);  // 0x709B60
    // The position on the node's timing, or zero when it has none. Names
    // not in the reference map.
    float GetTimingPosition();                // 0x709B70
    void SetTimingPosition(float position);   // 0x709BC0
    // The entity the graph runs for.
    Entity* _GetContextEntity() const;  // 0x709C70
    // Whether the instance runs the object's node or depends on the
    // object. Name not in the reference map.
    bool UsesObject(GameObjectId object) const;  // 0x709C80

    // Field names are not in the reference map.
    const StateNode* mNode;
    StateGraphInstance* mGraph;
    bool mEntered;
    // Set once _Poll reported the node finished.
    bool mFinished;
    bool mExited;
};

static_assert(offsetof(StateNodeInstance, mNode) == 8);
static_assert(offsetof(StateNodeInstance, mEntered) == 24);
static_assert(offsetof(StateNodeInstance, mExited) == 26);
static_assert(sizeof(StateNodeInstance) == 32);
