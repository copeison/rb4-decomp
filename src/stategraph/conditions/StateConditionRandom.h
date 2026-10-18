#pragma once

#include "stategraph/conditions/StateCondition.h"
#include "utl/text/Symbol.h"

// The condition that tests a random draw (stategraph/StateConditionRandom.o;
// its registration is at 0x6FD890, class id "StateConditionRandom"). Only
// the class symbols StateCondition::Create uses and the registration
// StateGraph::Init calls are declared; the class is not reconstructed. The
// object is 512 bytes.
class StateConditionRandom : public StateCondition {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FD890.
    static void Init();

    static Symbol sId;  // 0x1AC75A0
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AC75A8
};
