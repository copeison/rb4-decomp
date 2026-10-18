#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that sets the frame of an animation
// (stategraph/StateNodeSetAnimFrame.o; its registration is at 0x6F9F10,
// class id "StateNodeSetAnimFrame"). Only the class symbols and the
// registration StateGraph::Init calls are declared; the class is not
// reconstructed. The object is 480 bytes.
class StateNodeSetAnimFrame : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F9F10.
    static void Init();

    static Symbol sId;  // 0x1AB9410
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB9418
};
