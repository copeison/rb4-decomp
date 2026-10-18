#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// A node that holds other nodes and runs from its "start" node to its
// "exit" node (stategraph/StateNodeFolder.o, after the map's build; its
// registration is at 0x6F96C0, class id "StateNodeFolder"). Only the
// members the graph runtime reads and the registration are declared; the
// class is not reconstructed. The vtable is at 0x193BD48. The object is 176 bytes.
class StateNodeFolder : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F96C0.
    static void Init();

    static Symbol sId;  // 0x1AB3BA0
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB3BA8

    // Field names are not in the reference map; they follow the
    // properties.
    GameObjectId mStart;  // "start"
    GameObjectId mExit;   // "exit"
    // The nodes of mStart and mExit, resolved by _Fixup (0x71A120).
    StateNode* mStartNode;
    StateNode* mExitNode;
};

static_assert(offsetof(StateNodeFolder, mStart) == 152);
static_assert(offsetof(StateNodeFolder, mStartNode) == 160);
static_assert(sizeof(StateNodeFolder) == 176);
