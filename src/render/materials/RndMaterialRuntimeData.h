#pragma once

#include <cstddef>

#include "audio/core/resources/Resource.h"
#include "utl/containers/Vector.h"

class RndShaderCBuffer;
class RndTextureBase;

// Per-material GPU data: the shader graph it was built from, its constant
// buffer and the textures it holds. Released at a frame boundary once the
// GPU is done with it. The layout follows the constructor (0x4F72A0), the
// constant-buffer setup (0x4F7470), the texture loader (0x4F82D0) and the
// usage hints (0x4F8D90); field names are not in the reference map.
class RndMaterialRuntimeData {
public:
    ~RndMaterialRuntimeData();  // 0x4F7580

    // The device frame count when the data was last used.
    unsigned long mFrameStamp;
    // The map's constructor takes a RndShaderGraphResource*.
    ResourcePtr<Resource> mShaderGraph;
    // The shader-graph component on the resource's root object; its
    // exposed properties lay out the constant buffer.
    void* mGraph;
    // Three flags cached from virtual queries on the graph's root node
    // (vtable offsets 488, 512 and 520); likely among the map's IsLit,
    // UsesSceneTex and UsesSceneDepth. Not decoded.
    bool mRootFlags[3];
    // The material render state the usage hints read: a blend mode at 28
    // (-1 until set), a Hmx::Color at 32 that starts white, a mode at 48
    // and flags at 52-54. Not decoded member by member.
    unsigned char mRenderState[29];
    // One usage-hint mask per shader variant, built by InitUsageHints.
    unsigned int mUsageHints[3];
    RndShaderCBuffer* mCBuffer;
    // Constant offsets in the "Material" buffer: gVolumetricBlendParams,
    // then the first exposed property.
    unsigned long mVolumetricBlendParams;
    unsigned long mExposedProps;
    // Exposed textures by RndTextureBase::Type: 1D, 2D, cube and 2D array.
    eastl::vector<ResourcePtr<Resource>> mExposedTextures1D;
    eastl::vector<ResourcePtr<Resource>> mExposedTextures2D;
    eastl::vector<ResourcePtr<Resource>> mExposedTexturesCube;
    eastl::vector<ResourcePtr<Resource>> mExposedTexturesArray2D;
    // The texture bound to each of the graph's samplers, falling back to a
    // default texture.
    eastl::vector<RndTextureBase*> mSamplerTextures;
};

static_assert(offsetof(RndMaterialRuntimeData, mShaderGraph) == 8);
static_assert(offsetof(RndMaterialRuntimeData, mGraph) == 16);
static_assert(offsetof(RndMaterialRuntimeData, mUsageHints) == 56);
static_assert(offsetof(RndMaterialRuntimeData, mCBuffer) == 72);
static_assert(offsetof(RndMaterialRuntimeData, mVolumetricBlendParams) == 80);
static_assert(offsetof(RndMaterialRuntimeData, mExposedTextures1D) == 96);
static_assert(offsetof(RndMaterialRuntimeData, mExposedTexturesArray2D) == 192);
static_assert(offsetof(RndMaterialRuntimeData, mSamplerTextures) == 224);
