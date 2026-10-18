#pragma once

#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

// The node that plays clips through a ClipJukebox
// (stategraph/StateNodeClipJukebox.o, after the map's build; its
// registration is at 0x6F8DC0, class id "StateNodeClipJukebox"). Only the
// class symbols and the registration StateGraph::Init calls are declared;
// the class is not reconstructed. The object is 160 bytes.
class StateNodeClipJukebox : public StateNode {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F8DC0.
    static void Init();

    static Symbol sId;  // 0x1AB36C8
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1AB36D0
};
