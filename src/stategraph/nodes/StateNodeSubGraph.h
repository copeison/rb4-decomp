#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that runs another state graph inside it
// (stategraph/StateNodeSubGraph.o; its registration is at 0x6FA3B0, class id
// "StateNodeSubGraph"). Only the class symbols and the registration
// StateGraph::Init calls are declared; the class is not reconstructed. The
// object is 160 bytes.
class StateNodeSubGraph : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FA3B0.
    static void Init();

    static Symbol sId;  // 0x1ABD998
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1ABD9A0
};
