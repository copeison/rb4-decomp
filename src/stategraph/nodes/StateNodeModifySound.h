#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that modifies a sound the graph plays
// (stategraph/StateNodeModifySound.o; its registration is at 0x6FB2B0, class
// id "StateNodeModifySound"). Only the class symbols and the registration
// StateGraph::Init calls are declared; the class is not reconstructed. The
// object is 728 bytes.
class StateNodeModifySound : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FB2B0.
    static void Init();

    static Symbol sId;  // 0x1AB8CA8
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB8CB0
};
