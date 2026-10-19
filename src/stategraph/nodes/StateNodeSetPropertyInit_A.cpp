// stategraph/StateNodeSetPropertyInit_A.o (0x791400 to 0x793348): the
// registration of the StateNodeSetProperty<T> types float to unsigned char,
// with each type's Init and its factory and placement constructor
// (0x793170 on). The nodes' other code is emitted with the increment nodes
// in stategraph/StateNodeIncrementPropertyInit_A.o.
#include "stategraph/nodes/StateNodeSetProperty.h"

// Reconstructed from eboot.elf at 0x791400.
void StateNodeSetPropertyInit_A() {
    StateNodeSetProperty<float>::Init();
    StateNodeSetProperty<signed char>::Init();
    StateNodeSetProperty<short>::Init();
    StateNodeSetProperty<int>::Init();
    StateNodeSetProperty<long>::Init();
    StateNodeSetProperty<unsigned char>::Init();
}
