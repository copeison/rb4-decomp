#pragma once

#include "stategraph/conditions/StateCondition.h"
#include "utl/text/Symbol.h"

// The condition that tests the distance between two objects
// (stategraph/StateConditionObjectDistance.o, after the map's build; its
// registration is at 0x6FD4B0, class id "StateConditionObjectDistance").
// Only the class symbols StateCondition::Create uses and the registration
// StateGraph::Init calls are declared; the class is not reconstructed. The
// object is 528 bytes.
class StateConditionObjectDistance : public StateCondition {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FD4B0.
    static void Init();

    static Symbol sId;  // 0x1AC4640
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AC4648
};
