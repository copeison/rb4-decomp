#pragma once

#include <cstddef>

#include "math/rotation/Quat.h"
#include "math/vector/Vector3.h"

// A translation and a rotation quaternion (the map's QuatXfm; the
// animation code passes it, and no object of its own is in the map). New
// ones are the identity, as CharBoneKeyframeSet::Load (0x334F70) builds
// them. Field names are not in the reference map. The object is 28 bytes.
class QuatXfm {
public:
    QuatXfm() : v{0.0f, 0.0f, 0.0f}, q{0.0f, 0.0f, 0.0f, 1.0f} {}

    Vector3 v;
    Hmx::Quat q;
};

static_assert(offsetof(QuatXfm, q) == 12);
static_assert(sizeof(QuatXfm) == 28);
