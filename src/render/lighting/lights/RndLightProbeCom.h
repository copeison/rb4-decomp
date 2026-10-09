#pragma once

#include <cstddef>
#include <cstdint>

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

    // Field names are not in the reference map; the members take the names
    // of the properties the probe's registry (0x49B290) binds to their
    // offsets. The constructor is at 0x498170.
    // Alignment padding before mFalloffStart; the constructor does not
    // write it and no probe property lives there.
    unsigned char mPad;
    float mFalloffStart;
    float mFalloffEnd;
    std::int32_t mFalloffFunction;
    // Alignment padding before mStates; never written by the constructor.
    unsigned char mPad2[4];
    // An array object (the vtable at 0x1901938 that RndLightMgrCom's
    // "probe_lighting/states" also uses), presumably the probe's per-state
    // data. Its layout is not modelled; the name is uncertain.
    unsigned char mStates[40];
    // The "capture" group.
    float mRange;
    bool mIncludeAtmosphere;
    float mBackgroundColor[4];
    // Zeroed by the constructor and not decoded; likely the two filtered
    // capture cubes. The name is uncertain.
    void* mCaptureTextures[2];
    // Set when the probe's GPU data must be rebuilt.
    bool mDirty;
};

static_assert(offsetof(RndLightProbeCom, mFalloffStart) == 24);
static_assert(offsetof(RndLightProbeCom, mFalloffEnd) == 28);
static_assert(offsetof(RndLightProbeCom, mFalloffFunction) == 32);
static_assert(offsetof(RndLightProbeCom, mStates) == 40);
static_assert(offsetof(RndLightProbeCom, mRange) == 80);
static_assert(offsetof(RndLightProbeCom, mIncludeAtmosphere) == 84);
static_assert(offsetof(RndLightProbeCom, mBackgroundColor) == 88);
static_assert(offsetof(RndLightProbeCom, mCaptureTextures) == 104);
static_assert(offsetof(RndLightProbeCom, mDirty) == 120);
