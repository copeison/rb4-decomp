#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The base of the nodes that blend animations through a blend tree
// (stategraph/StateNodeBlendTreeBase.o, after the map's build; its
// registration is at 0x6F84A0, class id "StateNodeBlendTreeBase"). Only the
// class symbols the graph resource uses and the registration are declared;
// the class is not reconstructed. The object is 160 bytes.
class StateNodeBlendTreeBase : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F84A0.
    static void Init();

    static Symbol sId;  // 0x1AB3210
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB3218
};
