#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that triggers an action (stategraph/StateNodeTriggerAction.o,
// after the map's build; its registration is at 0x6FA820, class id
// "StateNodeTriggerAction"). Only the class symbols and the registration
// StateGraph::Init calls are declared; the class is not reconstructed. The
// object is 376 bytes.
class StateNodeTriggerAction : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FA820.
    static void Init();

    static Symbol sId;  // 0x1ABDC00
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1ABDC08
};
