// stategraph/StateNodeSetPropertyInit_B.o (0x793350 to 0x7A9AB3): the
// registration of the StateNodeSetProperty<T> types unsigned short to
// Symbol, with each type's Init and its factory and placement constructor.
// The bool, GameObjectId and Symbol nodes' code follows the Init functions,
// with their value nodes'; the other types' is emitted with the
// increment nodes in stategraph/StateNodeIncrementPropertyInit_B.o.
#include "stategraph/nodes/StateNodeSetProperty.h"

// Reconstructed from eboot.elf at 0x793350.
void StateNodeSetPropertyInit_B() {
    StateNodeSetProperty<unsigned short>::Init();
    StateNodeSetProperty<unsigned int>::Init();
    StateNodeSetProperty<unsigned long>::Init();
    StateNodeSetProperty<bool>::Init();
    StateNodeSetProperty<GameObjectId>::Init();
    StateNodeSetProperty<Symbol>::Init();
}
