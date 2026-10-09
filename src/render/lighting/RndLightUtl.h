#pragma once

#include "utl/text/Symbol.h"

// The light types, as RndLightCom::_GetTypeImpl returns them and the light
// manager indexes its light lists and buffers. The map names the type; the
// enumerator names are not in the reference map.
enum RndLightType : int {
    kRndLightPoint = 0,
    kRndLightSpot = 1,
    kRndLightDirectional = 2,
};

// Lighting helpers (render/RndLightUtl.o). Only what the light manager
// calls is declared.
class RndLightUtl {
public:
    // "point", "spot" or "directional"; the empty symbol for any other
    // value.
    static Symbol GetLightTypeName(RndLightType type);  // 0x4AB460
};
