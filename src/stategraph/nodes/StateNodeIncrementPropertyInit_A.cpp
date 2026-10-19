// stategraph/StateNodeIncrementPropertyInit_A.o (0x71E670 to 0x75F977;
// newer than the reference map, the name inferred from the property
// nodes'): the registration of the StateNodeIncrementProperty<T> types
// float to unsigned char. As the first user, the object holds these types'
// increment, property and value node code (the float value node's own
// methods are in stategraph/StateGraph.o) and GetPropRefRange (0x724CE0).
#include "stategraph/nodes/StateNodeIncrementProperty.h"

// Reconstructed from eboot.elf at 0x71E670.
void StateNodeIncrementPropertyInit_A() {
    StateNodeIncrementProperty<float>::Init();
    StateNodeIncrementProperty<signed char>::Init();
    StateNodeIncrementProperty<short>::Init();
    StateNodeIncrementProperty<int>::Init();
    StateNodeIncrementProperty<long>::Init();
    StateNodeIncrementProperty<unsigned char>::Init();
}
