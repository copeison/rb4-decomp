// stategraph/StateConditionPropertyComparisonInit_B.o (0x870010 to
// 0x881B7F): the registration of the next property comparisons and the
// template members the registrations instantiate, by type as in
// StateConditionPropertyComparisonInit_A.o:
//
// | T | vtable | Instance vtable | Init | _Init | sId | sClassName |
// | --- | --- | --- | --- | --- | --- | --- |
// | unsigned short | 0x1959138 | 0x1959320 | 0x870040 | 0x872500 | 0x1AC5B08 | 0x1AC5888 |
// | unsigned int | 0x1959560 | 0x1959748 | 0x8704A0 | 0x8755C0 | 0x1AC5D98 | 0x1AC5B18 |
// | unsigned long | 0x1959988 | 0x1959B70 | 0x870900 | 0x878620 | 0x1AC6028 | 0x1AC5DA8 |
// | bool | 0x1959DB0 | 0x1959F98 | 0x870D60 | 0x87B760 | 0x1AC62B8 | 0x1AC6038 |
// | GameObjectId | 0x195A180 | 0x195A368 | 0x8711C0 | 0x87D8B0 | 0x1AC6548 | 0x1AC62C8 |
// | Symbol | 0x195A550 | 0x195A738 | 0x871620 | 0x87FA10 | 0x1AC67D8 | 0x1AC6558 |
//
// The comparison instances of the unsigned types are here (vtables
// 0x1959378, 0x19597A0, 0x1959BC8); the others are in
// stategraph/StateConditionEventParameter.o.
#include "stategraph/conditions/StateConditionPropertyComparison.h"

// Reconstructed from eboot.elf at 0x870010.
void StateConditionPropertyComparisonInit_B() {
    StateConditionPropertyComparison<unsigned short>::Init();
    StateConditionPropertyComparison<unsigned int>::Init();
    StateConditionPropertyComparison<unsigned long>::Init();
    StateConditionPropertyComparison<bool>::Init();
    StateConditionPropertyComparison<GameObjectId>::Init();
    StateConditionPropertyComparison<Symbol>::Init();
}
