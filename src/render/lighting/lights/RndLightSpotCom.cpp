#include "render/lighting/lights/RndLightSpotCom.h"

// Reconstructed from eboot.elf at 0x49F690.
void RndLightSpotCom::SetFalloffStart(float distance) {
    mFalloffStart = distance;
    const float end = distance > mFalloffEnd ? distance : mFalloffEnd;
    const bool changed = end != mFalloffEnd;
    mFalloffEnd = end;
    if (changed) {
        _SyncGeometry();
        mDirty = true;
    }
}

// Reconstructed from eboot.elf at 0x49F720.
void RndLightSpotCom::SetFalloffEnd(float distance) {
    mFalloffEnd = distance;
    mFalloffStart = distance < mFalloffStart ? distance : mFalloffStart;
    _SyncGeometry();
    mDirty = true;
}
