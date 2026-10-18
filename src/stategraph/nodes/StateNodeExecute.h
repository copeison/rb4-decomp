#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that fires a script command (stategraph/StateNodeExecute.o; its
// registration is at 0x6F9250, class id "StateNodeExecute"). Only the class
// symbols and the registration StateGraph::Init calls are declared; the
// class is not reconstructed. The object is 168 bytes.
class StateNodeExecute : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F9250.
    static void Init();

    static Symbol sId;  // 0x1AB3940
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB3948
};
