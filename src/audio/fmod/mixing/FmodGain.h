#pragma once

#include "audio/fmod/api/fmod_api.h"

// Custom FMOD DSP plugin. Only the description getter, which FModSystem
// registers with each Studio system, is declared.
class FmodGainPlugin {
public:
    static FMOD_DSP_DESCRIPTION* GetDSPDescription();  // 0x27F0B0
};
