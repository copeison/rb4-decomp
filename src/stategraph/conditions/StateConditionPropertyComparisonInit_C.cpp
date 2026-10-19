// stategraph/StateConditionPropertyComparisonInit_C.o (0x881B80 to
// 0x88E53F): the registration of the last property comparisons and the
// template members the registrations instantiate, by type as in
// StateConditionPropertyComparisonInit_A.o:
//
// | T | vtable | Instance vtable | Init | _Init | sId | sClassName |
// | --- | --- | --- | --- | --- | --- | --- |
// | ResourcePath | 0x195A920 | 0x195AB08 | 0x881BA0 | 0x883A50 | 0x1AC6AF8 | 0x1AC6878 |
// | Hmx::Color | 0x195ACF0 | 0x195AED8 | 0x882000 | 0x885BC0 | 0x1AC6D88 | 0x1AC6B08 |
// | PropRef | 0x195B0C0 | 0x195B2A8 | 0x882470 | 0x887D70 | 0x1AC7018 | 0x1AC6D98 |
// | Waveform<float> | 0x195B490 | 0x195B678 | 0x8828D0 | 0x889DC0 | 0x1AC72A8 | 0x1AC7028 |
// | Waveform<Hmx::Color> | 0x195B860 | 0x195BA48 | 0x882D30 | 0x88C180 | 0x1AC7538 | 0x1AC72B8 |
//
// The comparison instances of these types are in
// stategraph/StateConditionEventParameter.o.
#include "stategraph/conditions/StateConditionPropertyComparison.h"

// Reconstructed from eboot.elf at 0x881B80.
void StateConditionPropertyComparisonInit_C() {
    StateConditionPropertyComparison<ResourcePath>::Init();
    StateConditionPropertyComparison<Hmx::Color>::Init();
    StateConditionPropertyComparison<PropRef>::Init();
    StateConditionPropertyComparison<Waveform<float>>::Init();
    StateConditionPropertyComparison<Waveform<Hmx::Color>>::Init();
}
