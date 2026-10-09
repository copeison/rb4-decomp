#pragma once

#include "audio/fmod/api/fmod_api.h"

// Custom FMOD DSP plugin. Only the description getter, which FModSystem
// registers with each Studio system, is declared.
// Its DSP is named "Hmx.SoundClashSlot".
// The map has no object for this plugin; the class name is not in the
// reference map.
class SoundClashSlotPlugin {
public:
    static FMOD_DSP_DESCRIPTION* GetDSPDescription();  // 0x280550
};
