#pragma once

#include <cstddef>

#include "utl/containers/FixedVector.h"

class RndMesh;

// The device's shared primitive meshes: a default box and a default
// cylinder. RndDevice allocates one in Init and deletes it in Terminate.
// Name not in the reference map.
class RndPrimitiveMeshes {
public:
    RndPrimitiveMeshes();   // 0x460640
    ~RndPrimitiveMeshes();  // 0x460880

    // The box, then the cylinder. Field name not in the reference map.
    FixedVector<RndMesh*, 2> mMeshes;
};

static_assert(offsetof(RndPrimitiveMeshes, mMeshes) == 0);
static_assert(sizeof(RndPrimitiveMeshes) == 40);
