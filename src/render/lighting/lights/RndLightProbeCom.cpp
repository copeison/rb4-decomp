#include "render/lighting/lights/RndLightProbeCom.h"

// Reconstructed from eboot.elf at 0x498440.
void RndLightProbeCom::SetFalloffStart(float distance) {
    mFalloffStart = distance;
    const float end = distance > mFalloffEnd ? distance : mFalloffEnd;
    const bool changed = end != mFalloffEnd;
    mFalloffEnd = end;
    if (changed) {
        mDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x498480.
void RndLightProbeCom::SetFalloffEnd(float distance) {
    mFalloffEnd = distance;
    mFalloffStart = distance < mFalloffStart ? distance : mFalloffStart;
    mDirty = true;
}
