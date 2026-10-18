#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The component that the base of the blend tree components, which mix
// animations by their parameters (class id "BlendTreeBase", after the map's
// build; its registration is at 0x6F7860). Only the registration
// StateGraph::Init calls and the class symbols are declared; the class is
// not reconstructed. The object is 24 bytes. Name not in the reference map;
// it follows the class id and the engine's "Com" suffix.
class BlendTreeBaseCom : public Component {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F7860.
    static void Init();

    static Symbol sId;  // 0x1ABE350, "BlendTreeBase"
    // Also "BlendTreeBase". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABE358
};
