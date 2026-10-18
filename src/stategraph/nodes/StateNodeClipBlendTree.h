#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that blends clips through a ClipBlendTree
// (stategraph/StateNodeClipBlendTree.o, after the map's build; its
// registration is at 0x6F8930, class id "StateNodeClipBlendTree"). Only the
// class symbols and the registration StateGraph::Init calls are declared;
// the class is not reconstructed. The object is 160 bytes.
class StateNodeClipBlendTree : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F8930.
    static void Init();

    static Symbol sId;  // 0x1AB3468
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB3470
};
