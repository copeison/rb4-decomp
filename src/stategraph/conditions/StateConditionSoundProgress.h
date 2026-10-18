#pragma once

#include "stategraph/conditions/StateCondition.h"
#include "utl/text/Symbol.h"

// The condition that tests the progress of a sound the graph plays
// (stategraph/StateConditionSoundProgress.o, after the map's build; its
// registration is at 0x6FDC70, class id "StateConditionSoundProgress"). Only
// the class symbols StateCondition::Create uses and the registration
// StateGraph::Init calls are declared; the class is not reconstructed. The
// object is 536 bytes.
class StateConditionSoundProgress : public StateCondition {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FDC70.
    static void Init();

    static Symbol sId;  // 0x1AC7800
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AC7808
};
