#pragma once

#include <cstddef>

#include "entity/core/DynamicCom.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

class RndShaderGraph;

// The resource of a shader graph file (render/RndShaderGraphResource.o): an
// entity whose root object holds the RndShaderGraph component, and the
// registry its exposed properties add to the materials that use it. Its
// methods are not reconstructed; only what the material component uses is
// declared, and the resource interface is left to the base.
class RndShaderGraphResource : public Resource {
public:
    // The class id, created on first use. The local static is at 0x1A8AD48
    // in render/RndMaterialCom.o.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("RndShaderGraphResource");
        }
        return id;
    }

    // The RndShaderGraph component on the entity's root object, or null.
    RndShaderGraph* GetShaderGraph();  // 0x50B110

    // The members from 48 to the registry; not modelled.
    unsigned char mMembers[160];
    // The registry the materials' dynamic properties use. Name not in the
    // reference map.
    DynamicPropRegistry<RndShaderGraphResource> mDynamicPropRegistry;
};

static_assert(offsetof(RndShaderGraphResource, mDynamicPropRegistry) == 208);
static_assert(offsetof(RndShaderGraphResource, mDynamicPropRegistry.mRegistry) == 224);
static_assert(offsetof(RndShaderGraphResource, mDynamicPropRegistry.mCrc) == 384);
static_assert(offsetof(RndShaderGraphResource, mDynamicPropRegistry.mDirty) == 400);
