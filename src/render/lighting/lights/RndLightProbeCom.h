#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// A light probe. Only the falloff setters, which the renderer's defaults
// call, are reconstructed; the probe is switched with the component's
// enabled flag.
class RndLightProbeCom : public Component {
public:
    // Sets where the falloff starts, pushing the end out to it.
    void SetFalloffStart(float distance);  // 0x498440
    // Sets where the falloff ends, pulling the start in to it.
    void SetFalloffEnd(float distance);  // 0x498480

    // The class symbol, "LightProbe", constructed by the static initializer
    // at 0x49DAE4.
    static Symbol sId;  // 0x1A88DE8

    // Field names are not in the reference map.
    unsigned char mUnknown23[1];
    float mFalloffStart;
    float mFalloffEnd;
    unsigned char mUnknown32[88];
    // Set when the probe's GPU data must be rebuilt.
    bool mDirty;
};

static_assert(offsetof(RndLightProbeCom, mFalloffStart) == 24);
static_assert(offsetof(RndLightProbeCom, mFalloffEnd) == 28);
static_assert(offsetof(RndLightProbeCom, mDirty) == 120);
