#pragma once

#include <cstddef>

#include "math/transform/Transform.h"

// The transform component (entity/Utl.o in the map). Its Component base and
// its methods are not reconstructed; only the class id and the world
// transform that the renderer reads are modelled.
class TransCom {
public:
    // Assigned when the component class registers; GetId (0x1B4CB0) returns
    // it.
    static unsigned long sId;  // 0x19E46E8

    // Field names are not in the reference map.
    unsigned char mUnknown0[80];
    Transform mWorldXfm;
};

static_assert(offsetof(TransCom, mWorldXfm) == 80);
