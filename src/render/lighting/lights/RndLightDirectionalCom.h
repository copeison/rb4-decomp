#pragma once

#include <cstddef>

#include "render/lighting/lights/RndLightCom.h"
#include "utl/text/Symbol.h"

// A directional light. Its methods are not reconstructed; only the class
// symbols and the light wrap that the renderer's backup lighting sets are
// declared.
class RndLightDirectionalCom : public RndLightCom {
public:
    // The class symbol, "LightDirectional", constructed by the static
    // initializer at 0x477240.
    static Symbol sId;  // 0x1A87638
    // The class's second symbol, with the same string, which the component
    // factory is given. Name not in the reference map.
    static Symbol sClassName;  // 0x1A87640

    // Field names are not in the reference map; the members take the names
    // of the properties RndLightCom's registry (0x46D760) binds to their
    // offsets.
    // RndLightCom's members, which RndLightCom does not declare yet: the
    // registry places "environments" at 24 and "color" at 64; byte 23 is
    // padding.
    unsigned char mLightComMembers[57];
    float mIntensity;
    int mIlluminationType;
    // Set to 2 for the backup light at 0x6BF5B6.
    float mLightWrap;
};

static_assert(offsetof(RndLightDirectionalCom, mIntensity) == 80);
static_assert(offsetof(RndLightDirectionalCom, mIlluminationType) == 84);
static_assert(offsetof(RndLightDirectionalCom, mLightWrap) == 88);
