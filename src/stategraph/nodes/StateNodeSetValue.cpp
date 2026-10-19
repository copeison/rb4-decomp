// stategraph/StateNodeSetValue.o (0x7E9320 to 0x7F2307): the registration
// of the StateNodeSetValue<T> types, with each type's Init (0x7E9390 to
// 0x7EFBC7), the guarded statics' initializers and each type's factory and
// placement constructor (0x7EFF90 on). The nodes' other code is emitted
// with the property nodes (stategraph/StateNodeIncrementPropertyInit_A.o
// and on) and, for float, in stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeSetValue.h"

// Reconstructed from eboot.elf at 0x7E9320.
void StateNodeSetValueInit() {
    StateNodeSetValue<float>::Init();
    StateNodeSetValue<signed char>::Init();
    StateNodeSetValue<short>::Init();
    StateNodeSetValue<int>::Init();
    StateNodeSetValue<long>::Init();
    StateNodeSetValue<unsigned char>::Init();
    StateNodeSetValue<unsigned short>::Init();
    StateNodeSetValue<unsigned int>::Init();
    StateNodeSetValue<unsigned long>::Init();
    StateNodeSetValue<bool>::Init();
    StateNodeSetValue<GameObjectId>::Init();
    StateNodeSetValue<Symbol>::Init();
    StateNodeSetValue<ResourcePath>::Init();
    StateNodeSetValue<Hmx::Color>::Init();
    StateNodeSetValue<PropRef>::Init();
    StateNodeSetValue<Waveform<float>>::Init();
    StateNodeSetValue<Waveform<Hmx::Color>>::Init();
    StateNodeSetValue<Vector3>::Init();
    StateNodeSetValue<PropRotation>::Init();
    StateNodeSetValue<PropXfm>::Init();
}
