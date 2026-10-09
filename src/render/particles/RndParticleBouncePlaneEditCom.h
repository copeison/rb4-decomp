#pragma once

#include "entity/core/Component.h"

// The editor counterpart of the particle bounce plane ("RndEditorDrawCom
// counterpart to RndParticleColliderCom", registered at 0x600170). Only the
// static mesh entry points that Rnd::Init and Rnd::Terminate call are
// declared; they follow its registration in the binary, which places them
// with this class.
class RndParticleBouncePlaneEditCom : public Component {
public:
    // Builds the shared box mesh. Name not in the reference map.
    static void StaticInit();  // 0x601880
    // Name not in the reference map.
    static void StaticTerminate();  // 0x601910
};
