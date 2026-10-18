#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that runs its child nodes together, in sequence or one picked at
// random (stategraph/StateNodeGroup.o; its registration is at 0x6F9B30,
// class id "StateNodeGroup"). Only the class symbols and the registration
// StateGraph::Init calls are declared; the class is not reconstructed. The
// object is 232 bytes.
class StateNodeGroup : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F9B30.
    static void Init();

    static Symbol sId;  // 0x1AB3E00
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB3E08
};
