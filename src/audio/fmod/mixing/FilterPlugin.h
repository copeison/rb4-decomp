#pragma once

#include "audio/fmod/api/fmod_api.h"

// Custom FMOD DSP plugin. Only the description getter, which FModSystem
// registers with each Studio system, is declared.
class FilterPlugin {
public:
    static FMOD_DSP_DESCRIPTION* GetDSPDescription();  // 0x27E7A0
};
