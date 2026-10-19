// stategraph/StateNodeIncrementPropertyInit_B.o (0x75F980 to 0x780B08;
// newer than the reference map, the name inferred from the property
// nodes'): the registration of the StateNodeIncrementProperty<T> types
// unsigned short to unsigned long. As the first user, the object holds
// these types' increment, property and value node code.
#include "stategraph/nodes/StateNodeIncrementProperty.h"

// Reconstructed from eboot.elf at 0x75F980.
void StateNodeIncrementPropertyInit_B() {
    StateNodeIncrementProperty<unsigned short>::Init();
    StateNodeIncrementProperty<unsigned int>::Init();
    StateNodeIncrementProperty<unsigned long>::Init();
}
