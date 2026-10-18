#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

// A skeleton's bones (anim/CharSkeleton.o). Only the bone names, which
// RndMeshCom's "root_bone" property reads, are declared; the class is not
// reconstructed. Field names are not in the reference map.
class CharSkeleton {
public:
    // The members before the names; not decoded.
    unsigned char mMembers[96];
    // The bone names, by bone index (the map's SetNames sets them).
    eastl::vector<Symbol> mNames;
};

static_assert(offsetof(CharSkeleton, mNames) == 96);
