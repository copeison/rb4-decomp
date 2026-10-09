#pragma once

#include <cstddef>

#include "audio/core/resources/Resource.h"
#include "utl/containers/Vector.h"

class RndShaderCBuffer;

// Per-material GPU data: the shader graph it was built from, its constant
// buffer and the resources it holds. Released at a frame boundary once the
// GPU is done with it. Only the members the destructor touches are
// modelled; field names are not in the reference map.
class RndMaterialRuntimeData {
public:
    ~RndMaterialRuntimeData();  // 0x4F7580

    // The device frame count when the data was last used.
    unsigned long mFrameStamp;
    // The map's constructor takes a RndShaderGraphResource*.
    ResourcePtr<Resource> mShaderGraph;
    unsigned char mUnknown16[56];
    RndShaderCBuffer* mCBuffer;
    unsigned char mUnknown80[16];
    // Held resource references.
    eastl::vector<ResourcePtr<Resource>> mUnknown96;
    eastl::vector<ResourcePtr<Resource>> mUnknown128;
    eastl::vector<ResourcePtr<Resource>> mUnknown160;
    eastl::vector<ResourcePtr<Resource>> mUnknown192;
    // Element type unknown; it has a trivial destructor.
    eastl::vector<unsigned char> mUnknown224;
};

static_assert(offsetof(RndMaterialRuntimeData, mShaderGraph) == 8);
static_assert(offsetof(RndMaterialRuntimeData, mCBuffer) == 72);
static_assert(offsetof(RndMaterialRuntimeData, mUnknown96) == 96);
static_assert(offsetof(RndMaterialRuntimeData, mUnknown192) == 192);
static_assert(offsetof(RndMaterialRuntimeData, mUnknown224) == 224);
