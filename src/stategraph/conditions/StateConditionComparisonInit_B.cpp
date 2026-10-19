// stategraph/StateConditionComparisonInit_B.o (0x81E630 to 0x83199F): the
// registration of the comparisons of the other property types and the
// template members the registrations instantiate, by type as in
// StateConditionComparisonInit_A.o:
//
// | T | vtable | Init | _Init | sId | sClassName |
// | --- | --- | --- | --- | --- | --- |
// | bool | 0x1952E60 | 0x81E660 | 0x821620 | 0x1AC0CF8 | 0x1AC0A80 |
// | GameObjectId | 0x1953228 | 0x81EA90 | 0x823510 | 0x1AC0F88 | 0x1AC0D08 |
// | Symbol | 0x19535F0 | 0x81EEC0 | 0x825410 | 0x1AC1218 | 0x1AC0F98 |
// | ResourcePath | 0x19539B8 | 0x81F2F0 | 0x827310 | 0x1AC14A8 | 0x1AC1228 |
// | Hmx::Color | 0x1953D80 | 0x81F720 | 0x829210 | 0x1AC1738 | 0x1AC14B8 |
// | PropRef | 0x1954148 | 0x81FB60 | 0x82B120 | 0x1AC19C8 | 0x1AC1748 |
// | Waveform<float> | 0x1954510 | 0x81FFA0 | 0x82D0C0 | 0x1AC1C58 | 0x1AC19D8 |
// | Waveform<Hmx::Color> | 0x19548D8 | 0x8203E0 | 0x82F530 | 0x1AC1EE8 | 0x1AC1C68 |
//
// The waveform prototypes are built out of line (0x82EAC0, 0x830F30).
#include "stategraph/conditions/StateConditionComparison.h"

// Reconstructed from eboot.elf at 0x81E630.
void StateConditionComparisonInit_B() {
    StateConditionComparison<bool>::Init();
    StateConditionComparison<GameObjectId>::Init();
    StateConditionComparison<Symbol>::Init();
    StateConditionComparison<ResourcePath>::Init();
    StateConditionComparison<Hmx::Color>::Init();
    StateConditionComparison<PropRef>::Init();
    StateConditionComparison<Waveform<float>>::Init();
    StateConditionComparison<Waveform<Hmx::Color>>::Init();
}
