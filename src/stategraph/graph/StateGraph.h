#pragma once

#include "entity/core/ComMetaData.h"
#include "stategraph/nodes/StateNodePlaySound.h"
#include "utl/text/Symbol.h"

// StateGraph::Init registers DrivenAudioParamSettings (declared with
// StateNodePlaySound, whose "params" hold it) through
// RegisterPropStruct<DrivenAudioParamSettings>, which the binary emits in
// stategraph/StateGraph.o at 0x6FB100.

// The state graph module (stategraph/StateGraph.o, 0x6F6090-0x7013A0): its
// Init registers the graph resource, the driver and every graph component
// class. The object also emits the classes' factories and the inline
// methods the templated node and condition classes share.
class StateGraph {
public:
    // App::Initialize calls it with the other modules' registrations.
    static void Init();  // 0x6F6090
};
