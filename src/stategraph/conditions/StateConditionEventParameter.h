#pragma once

#include "utl/text/Symbol.h"

// The condition that compares a parameter of an event with a value, for
// each value type. Only the class symbol of the float instantiation, which
// StateCondition::Create uses, is declared: a guarded template static
// (0x1ABF040) that stategraph/StateCondition.o sets to
// "StateConditionEventParameter_<type>" (0x8098C0).
template <class T>
class StateConditionEventParameter {
public:
    static Symbol sId;
};

// The registration of the StateConditionEventParameter<T> condition types
// (stategraph/StateConditionEventParameter.o): it calls each type's Init.
// Not reconstructed.
void StateConditionEventParameterInit();  // 0x835D40
