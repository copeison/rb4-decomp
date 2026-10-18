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

// The edge in texels of the spot shadow maps at the resolution index (256,
// 512, 1024, 1600, 2048, 3200 or 4096); -1 for any other index. Emitted
// after GetLightTypeName; the light manager's "spotlight_shadowmap_resolution"
// labels call it, and the spot shadow depth array inlines it. Name not in the
// reference map.
int RndSpotShadowResolution(unsigned int index);  // 0x4AB4A0
