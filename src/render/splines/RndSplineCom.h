#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The spline component (render/RndSplineCom.o). Its class id is "Spline".
// Only what RndMeshCom's "make_spline_fan" utility reads is declared: the
// class is not reconstructed, and its base and layout are not decoded.
class RndSplineCom : public Component {
public:
    // The class symbol, constructed by the object's static initializer at
    // 0x6514F0.
    static Symbol sId;  // 0x1AABB30
};
