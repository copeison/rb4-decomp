#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The component that plays one animation clip after another, crossfading,
// for the StateNodeClipJukebox nodes (class id "ClipJukebox", after the
// map's build; its registration is at 0x6F8060). Only the registration
// StateGraph::Init calls and the class symbols are declared; the class is
// not reconstructed. The object is 304 bytes. Name not in the reference map;
// it follows the class id and the engine's "Com" suffix.
class ClipJukeboxCom : public Component {
public:
    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6F8060.
    static void Init();

    static Symbol sId;  // 0x1ABEA70, "ClipJukebox"
    // Also "ClipJukebox". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABEA78
};
