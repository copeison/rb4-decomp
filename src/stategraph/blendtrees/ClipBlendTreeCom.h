#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The component that blends animation clips by a parameter, for the
// StateNodeClipBlendTree nodes (class id "ClipBlendTree", after the map's
// build; its registration is at 0x6F7C20). Only the registration
// StateGraph::Init calls and the class symbols are declared; the class is
// not reconstructed. The object is 304 bytes. Name not in the reference map;
// it follows the class id and the engine's "Com" suffix.
class ClipBlendTreeCom : public Component {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F7C20.
    static void Init();

    static Symbol sId;  // 0x1ABE6B0, "ClipBlendTree"
    // Also "ClipBlendTree". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABE6B8
};
