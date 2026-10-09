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
    unsigned char mUnknown23[65];
    // Set to 2 for the backup light at 0x6BF5B6.
    float mIntensity;
};

static_assert(offsetof(RndLightDirectionalCom, mIntensity) == 88);
