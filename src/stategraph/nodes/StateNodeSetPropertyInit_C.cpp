// stategraph/StateNodeSetPropertyInit_C.o (0x7A9AC0 to 0x7E9313): the
// registration of the StateNodeSetProperty<T> types ResourcePath to
// PropXfm, with each type's Init and its factory and placement
// constructor, and all of these nodes' code and their value nodes'.
#include "stategraph/nodes/StateNodeSetProperty.h"

// Reconstructed from eboot.elf at 0x7A9AC0.
void StateNodeSetPropertyInit_C() {
    StateNodeSetProperty<ResourcePath>::Init();
    StateNodeSetProperty<Hmx::Color>::Init();
    StateNodeSetProperty<PropRef>::Init();
    StateNodeSetProperty<Waveform<float>>::Init();
    StateNodeSetProperty<Waveform<Hmx::Color>>::Init();
    StateNodeSetProperty<Vector3>::Init();
    StateNodeSetProperty<PropRotation>::Init();
    StateNodeSetProperty<PropXfm>::Init();
}
