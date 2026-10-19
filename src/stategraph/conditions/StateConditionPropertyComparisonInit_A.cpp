// stategraph/StateConditionPropertyComparisonInit_A.o (0x85D630 to
// 0x87000F): the registration of the first property comparisons and the
// template members the registrations instantiate. Each type's block holds
// its Init (inlined into the registration), its static initializers, _Init,
// factory, lambdas, virtual methods and instance; the instances of the
// comparisons the object instantiates first follow theirs:
//
// | T | vtable | Instance vtable | Init | _Init | sId | sClassName |
// | --- | --- | --- | --- | --- | --- | --- |
// | float | 0x19578F8 | 0x1957AE0 | 0x85D660 | 0x85FA90 | 0x1AC4B08 | 0x1ABF050 |
// | signed char | 0x1957CC8 | 0x1957EB0 | 0x85DAC0 | 0x861BD0 | 0x1AC4D98 | 0x1AC4B18 |
// | short | 0x19580F0 | 0x19582D8 | 0x85DF20 | 0x864C80 | 0x1AC5028 | 0x1AC4DA8 |
// | int | 0x1958518 | 0x1958700 | 0x85E380 | 0x867D40 | 0x1AC52B8 | 0x1AC5038 |
// | long | 0x19588E8 | 0x1958AD0 | 0x85E7E0 | 0x869E90 | 0x1AC5548 | 0x1AC52C8 |
// | unsigned char | 0x1958D10 | 0x1958EF8 | 0x85EC40 | 0x86CF60 | 0x1AC57D8 | 0x1AC5558 |
//
// The comparison instances of signed char, short, long and unsigned char
// are here (vtables 0x1957F08, 0x1958330, 0x1958B28, 0x1958F50); those of
// float and int are in stategraph/StateConditionEventParameter.o.
#include "stategraph/conditions/StateConditionPropertyComparison.h"

// Reconstructed from eboot.elf at 0x85D630.
void StateConditionPropertyComparisonInit_A() {
    StateConditionPropertyComparison<float>::Init();
    StateConditionPropertyComparison<signed char>::Init();
    StateConditionPropertyComparison<short>::Init();
    StateConditionPropertyComparison<int>::Init();
    StateConditionPropertyComparison<long>::Init();
    StateConditionPropertyComparison<unsigned char>::Init();
}
