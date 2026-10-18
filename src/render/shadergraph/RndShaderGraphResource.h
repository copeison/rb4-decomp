#pragma once

#include <cstddef>

#include "entity/core/DynamicCom.h"
#include "entity/core/EntityResource.h"
#include "utl/text/Symbol.h"

class RndShaderGraph;

// The resource of a shader graph file (render/RndShaderGraphResource.o): an
// entity whose root object holds the RndShaderGraph component, and the
// registry its exposed properties add to the materials that use it. The base
// is EntityResource: the map's overrides (CreateEntity, InitObject) are
// its virtuals, and the registry follows its 200 bytes. Only what the
// material component uses is declared; GetShaderGraph is reconstructed.
class RndShaderGraphResource : public EntityResource {
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

    // +200 to the registry; not modelled.
    unsigned char mMembers200[8];
    // The registry the materials' dynamic properties use. Name not in the
    // reference map.
    DynamicPropRegistry<RndShaderGraphResource> mDynamicPropRegistry;
};

static_assert(offsetof(RndShaderGraphResource, mDynamicPropRegistry) == 208);
static_assert(offsetof(RndShaderGraphResource, mDynamicPropRegistry.mRegistry) == 224);
static_assert(offsetof(RndShaderGraphResource, mDynamicPropRegistry.mCrc) == 384);
static_assert(offsetof(RndShaderGraphResource, mDynamicPropRegistry.mDirty) == 400);
