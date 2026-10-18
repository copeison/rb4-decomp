#pragma once

#include "stategraph/conditions/StateCondition.h"
#include "utl/text/Symbol.h"

// The condition that tests the time the transition's scope has run
// (stategraph/StateConditionTimeElapsed.o; its registration is at 0x6FE050,
// class id "StateConditionTimeElapsed"). Only the class symbols
// StateCondition::Create uses and the registration StateGraph::Init calls
// are declared; the class is not reconstructed. The object is 520 bytes.
class StateConditionTimeElapsed : public StateCondition {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FE050.
    static void Init();

    static Symbol sId;  // 0x1AC7A60
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AC7A68
};
