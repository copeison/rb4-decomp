#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that plays an animation on the graph's context entity
// (stategraph/StateNodeAnimate.o; its registration is at 0x6F73C0, class id
// "StateNodeAnimate"). Only the class symbols and the registration
// StateGraph::Init calls are declared; the class is not reconstructed. The
// object is 2344 bytes.
class StateNodeAnimate : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F73C0.
    static void Init();

    static Symbol sId;  // 0x1AB2F30
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB2F38
};
