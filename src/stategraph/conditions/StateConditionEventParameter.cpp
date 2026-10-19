// stategraph/StateConditionEventParameter.o (0x835D40 to 0x85968A): the
// registration of the parameter comparisons and, through it, every member
// of StateConditionEventParameter<T> and of the StateConditionComparison<T>
// parts the object emits for the ten parameter types.
#include "stategraph/conditions/StateConditionEventParameter.h"

// Reconstructed from eboot.elf at 0x835D40.
void StateConditionEventParameterInit() {
    StateConditionEventParameter<float>::Init();
    StateConditionEventParameter<int>::Init();
    StateConditionEventParameter<bool>::Init();
    StateConditionEventParameter<GameObjectId>::Init();
    StateConditionEventParameter<Symbol>::Init();
    StateConditionEventParameter<ResourcePath>::Init();
    StateConditionEventParameter<Hmx::Color>::Init();
    StateConditionEventParameter<PropRef>::Init();
    StateConditionEventParameter<Waveform<float>>::Init();
    StateConditionEventParameter<Waveform<Hmx::Color>>::Init();
}
