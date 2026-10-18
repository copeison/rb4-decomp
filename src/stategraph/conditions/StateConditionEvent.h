#pragma once

#include "stategraph/conditions/StateCondition.h"
#include "utl/text/Symbol.h"

// The condition that tests whether the entity sent an event
// (stategraph/StateConditionEvent.o; its registration is at 0x6FC410, class
// id "StateConditionEvent"). Only the class symbols StateCondition::Create
// uses and the registration StateGraph::Init calls are declared; the class
// is not reconstructed. The object is 192 bytes.
class StateConditionEvent : public StateCondition {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FC410.
    static void Init();

    static Symbol sId;  // 0x1AC21A8
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AC21B0
};
