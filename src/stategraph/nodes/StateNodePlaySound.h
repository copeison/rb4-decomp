#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that plays a sound (stategraph/StateNodePlaySound.o; its
// registration is at 0x6FB760, class id "StateNodePlaySound"). Only the
// class symbols and the registration StateGraph::Init calls are declared;
// the class is not reconstructed. The object is 728 bytes.
class StateNodePlaySound : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FB760.
    static void Init();

    static Symbol sId;  // 0x1AB8F10
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB8F18
};
