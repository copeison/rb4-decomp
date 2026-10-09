#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class RndTextureCube;

// A cube texture loaded from a file or inlined into an entity resource
// (render/RndTextureCubeResource.o). Only the class id and the texture are
// modelled; the virtuals are not recovered. Light probes inline one per
// state and texture, and point lights load their cookies as one. The
// constructor at 0x6A3900 makes 832-byte objects.
class RndTextureCubeResource : public Resource {
public:
    // The class id, created on first use and inlined into its users, for
    // example Resource::GetOrLoad<RndTextureCubeResource> at 0x4924E0.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RndTextureCubeResource");
        }
        return id;
    }

    // The base part from 48 to 199, which the constructor builds through
    // the 200-byte base constructor IDA names EntityResource::EntityResource,
    // is not modelled. Name not in the reference map.
    unsigned char mBaseData[152];
    // The loaded texture, cleared by the constructor; RndLightProbeCom and
    // RndLightPointCom read it once the resource has not failed. Name not in
    // the reference map.
    RndTextureCube* mTexture;
};

static_assert(offsetof(RndTextureCubeResource, mTexture) == 200);
