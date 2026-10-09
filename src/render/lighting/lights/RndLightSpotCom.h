#pragma once

#include <cstddef>

#include "render/lighting/lights/RndLightCom.h"
#include "utl/text/Symbol.h"

// A spot light. Only the falloff setters, which the renderer's defaults
// call, are reconstructed; the other members are declared as the setters
// and RndDefaults use them.
class RndLightSpotCom : public RndLightCom {
public:
    // Sets where the falloff starts, pushing the end out to it.
    void SetFalloffStart(float distance);  // 0x49F690
    // Sets where the falloff ends, pulling the start in to it.
    void SetFalloffEnd(float distance);  // 0x49F720

    // Rebuilds the light volume from the angles and falloff.
    void _SyncGeometry();  // 0x4A28A0

    // The class symbol, "LightSpot", constructed by the static initializer
    // at 0x4A8620.
    static Symbol sId;  // 0x1A892F8

    // Field names are not in the reference map.
    unsigned char mUnknown23[213];
    float mFalloffStart;
    float mFalloffEnd;
    unsigned char mUnknown244[76];
    // Returned by RndDefaults::GetLightingShadowOffset.
    float mShadowOffset;
    unsigned char mUnknown324[28];
    // Set when the light's GPU data must be rebuilt.
    bool mDirty;
};

static_assert(offsetof(RndLightSpotCom, mFalloffStart) == 236);
static_assert(offsetof(RndLightSpotCom, mFalloffEnd) == 240);
static_assert(offsetof(RndLightSpotCom, mShadowOffset) == 320);
static_assert(offsetof(RndLightSpotCom, mDirty) == 352);
