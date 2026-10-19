// stategraph/StateConditionComparisonInit_A.o (0x809AA0 to 0x81E62F): the
// registration of the number comparisons and the template members the
// registrations instantiate. Each type's block holds its Init (inlined into
// the registration), its static initializers, _Init, factory, lambdas and
// virtual methods:
//
// | T | vtable | Init | _Init | sId | sClassName |
// | --- | --- | --- | --- | --- | --- |
// | float | 0x1950C58 | 0x809AE0 | 0x80CEC0 | 0x1AB2208 | 0x1AB2218 |
// | signed char | 0x1951020 | 0x809F10 | 0x80EE50 | 0x1ABF5D8 | 0x1ABF360 |
// | short | 0x19513E8 | 0x80A340 | 0x810D40 | 0x1ABF868 | 0x1ABF5E8 |
// | int | 0x19517B0 | 0x80A770 | 0x812C40 | 0x1ABFAF8 | 0x1ABF878 |
// | long | 0x1951B78 | 0x80ABA0 | 0x814B30 | 0x1ABFD88 | 0x1ABFB08 |
// | unsigned char | 0x1951F40 | 0x80AFD0 | 0x816A40 | 0x1AC0018 | 0x1ABFD98 |
// | unsigned short | 0x1952308 | 0x80B400 | 0x818930 | 0x1AC02A8 | 0x1AC0028 |
// | unsigned int | 0x19526D0 | 0x80B830 | 0x81A830 | 0x1AC0538 | 0x1AC02B8 |
// | unsigned long | 0x1952A98 | 0x80BC60 | 0x81C720 | 0x1AC07C8 | 0x1AC0548 |
//
// The float statics are initialized by an earlier object that uses the
// class. The object also holds a copy of StateCondition::_InitAsSuperclass
// (0x80D9E0).
#include "stategraph/conditions/StateConditionComparison.h"

// Reconstructed from eboot.elf at 0x809AA0.
void StateConditionComparisonInit_A() {
    StateConditionComparison<float>::Init();
    StateConditionComparison<signed char>::Init();
    StateConditionComparison<short>::Init();
    StateConditionComparison<int>::Init();
    StateConditionComparison<long>::Init();
    StateConditionComparison<unsigned char>::Init();
    StateConditionComparison<unsigned short>::Init();
    StateConditionComparison<unsigned int>::Init();
    StateConditionComparison<unsigned long>::Init();
}
