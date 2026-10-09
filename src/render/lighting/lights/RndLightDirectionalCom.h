#pragma once

#include <cstddef>

#include "render/lighting/lights/RndLightCom.h"
#include "utl/text/Symbol.h"

// A directional light. Its methods are not reconstructed; only the class
// symbols and the intensity that the renderer's backup lighting sets are
// declared.
class RndLightDirectionalCom : public RndLightCom {
public:
    // The class symbol, "LightDirectional", constructed by the static
    // initializer at 0x477240.
    static Symbol sId;  // 0x1A87638
    // The class's second symbol, with the same string, which the component
    // factory is given. Name not in the reference map.
    static Symbol sClassName;  // 0x1A87640

    // Field names are not in the reference map.
    // RndLightCom's members, which RndLightCom does not declare yet. Its
    // property registry (0x46D760) places "environments" at 24, "color" at
    // 64, "intensity" at 80 and "illumination_type" at 84; byte 23 is
    // padding.
    unsigned char mLightComMembers[65];
    // Set to 2 for the backup light at 0x6BF5B6. The RndLightCom registry
    // names offset 88 "light_wrap", so this may be the light wrap rather
    // than the intensity.
    float mIntensity;
};

static_assert(offsetof(RndLightDirectionalCom, mIntensity) == 88);
