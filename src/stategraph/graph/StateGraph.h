#pragma once

#include "entity/core/ComMetaData.h"
#include "utl/text/Symbol.h"

// The settings struct of the driven audio parameters that the sound nodes
// hold (304 bytes, 8-aligned), registered as a property struct by
// StateGraph::Init. Its layout is not modelled. Name from the struct's
// registered type name.
struct DrivenAudioParamSettings;

// Names the struct type and records its size, once (0x6FB100, emitted in
// stategraph/StateGraph.o). Not reconstructed: the struct is not modelled.
template <>
void RegisterPropStruct<DrivenAudioParamSettings>(Symbol name);

// The state graph module (stategraph/StateGraph.o, 0x6F6090-0x7013A0): its
// Init registers the graph resource, the driver and every graph component
// class. The object also emits the classes' factories and the inline
// methods the templated node and condition classes share.
class StateGraph {
public:
    // App::Initialize calls it with the other modules' registrations.
    static void Init();  // 0x6F6090
};
