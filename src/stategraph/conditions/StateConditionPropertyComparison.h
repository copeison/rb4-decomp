#pragma once

#include "utl/text/Symbol.h"

// The condition that compares a property with a value, for each property
// type. Only the class symbol of the float instantiation, which
// StateCondition::Create uses, is declared: a guarded template static
// (0x1ABF050) that stategraph/StateCondition.o initializes (0x809950).
template <class T>
class StateConditionPropertyComparison {
public:
    static Symbol sId;
};

// The registrations of the StateConditionPropertyComparison<T> condition
// types (stategraph/StateConditionPropertyComparisonInit_A.o to _C.o):
// each calls its types' Init. Not reconstructed.
void StateConditionPropertyComparisonInit_A();  // 0x85D630
void StateConditionPropertyComparisonInit_B();  // 0x870010
void StateConditionPropertyComparisonInit_C();  // 0x881B80
