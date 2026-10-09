#pragma once

#include <cstddef>

#include "audio/core/resources/Resource.h"
#include "utl/containers/Vector.h"

class RndComputeBuffer;
class RndMesh;
class RndCShaderTiledLightsApplication;
class RndCShaderTiledLightsCull;
class RndCShaderTiledLightsInterpolation;
class RndCShaderTiledLightsStereoToMono;
class RndCShaderTonemap;
class RndLightDirectionalDeferredShader;
class RndLightPointDeferredShader;
class RndLightProbeDeferredAccumShader;
class RndLightProbeDeferredShader;
class RndLightSpotDeferredShader;
class RndShaderLightDirectionalShadowGen;
class RndShaderLightPointShadowGen;
class RndShaderLightSpotShadowGen;
class RndTonemapShader;
class RndTextureBase;
struct RndInitParams;

// The renderer's shared lighting resources: the light-volume meshes, the
// lighting shaders, the tiled-light count buffer, the lighting textures, and
// the probe-capture textures. Embedded in RndDevice.
class RndLightGlobals {
public:
    RndLightGlobals();   // 0x47EEE0
    ~RndLightGlobals();  // 0x47EFA0

    // Inlines _InitMeshes and _InitBuffers.
    void Init();       // 0x47F030
    void Terminate();  // 0x47F580
    // Not reconstructed yet. The map's signature is LoadResources(); this
    // build reads RndInitParams::mInitRendering.
    void LoadResources(const RndInitParams& params);  // 0x47F8D0

    // The capture textures are found by width. Which getter reads which
    // vector follows the map's function order.
    RndTextureBase* GetProbeCaptureDownsampleTexture(int size);  // 0x47FD60
    RndTextureBase* GetProbeCaptureHelperTexture(int size);      // 0x47FDA0
    // Not reconstructed yet. The return type is not in the reference map.
    ResourcePath GetSkinDiffusionTexPath();  // 0x47FCD0

    void _InitBuffers();  // 0x47F500
    // Not reconstructed yet.
    void _InitLightSpotMesh();  // 0x47FDE0
    // Not reconstructed yet.
    void _InitMeshes();  // 0x47F1C0
    // Creates and registers the lighting and tonemap shaders; the tiled
    // compute shaders only on platforms with async compute.
    void _InitShaders();  // 0x47F300

    // Field names are not in the reference map.
    RndMesh* mSphereMesh;     // "lighting_sphere"
    RndMesh* mSpotlightMesh;  // Created by _InitLightSpotMesh.
    // Derived by _InitMeshes from the sphere's tessellation.
    float mSphereScale;
    unsigned long mPrimaryGroupSize;
    unsigned long mSecondaryGroupSize;
    RndLightDirectionalDeferredShader* mDirectionalShader;
    RndShaderLightDirectionalShadowGen* mDirectionalShadowGenShader;
    RndLightPointDeferredShader* mPointShader;
    RndShaderLightPointShadowGen* mPointShadowGenShader;
    RndLightSpotDeferredShader* mSpotShader;
    RndShaderLightSpotShadowGen* mSpotShadowGenShader;
    RndLightProbeDeferredShader* mProbeShader;
    RndLightProbeDeferredAccumShader* mProbeAccumShader;
    RndComputeBuffer* mTiledLightIdsCount;
    RndCShaderTiledLightsCull* mTiledLightsCullShader;
    RndCShaderTiledLightsApplication* mTiledLightsApplicationShader;
    RndCShaderTiledLightsInterpolation* mTiledLightsInterpolationShader;
    RndCShaderTiledLightsStereoToMono* mTiledLightsStereoToMonoShader;
    // Lazily loaded error_light_cookie.png.
    ResourcePtr<Resource> mErrorLightCookie;
    // inline_lighting_textures.entity.
    ResourcePtr<Resource> mInlineLightingTextures;
    // Read from mInlineLightingTextures by LoadResources.
    void* mInlineLightingData[2];
    // skin_diffusion.bmp.
    ResourcePtr<Resource> mSkinDiffusion;
    RndTonemapShader* mTonemapShader;
    RndCShaderTonemap* mTonemapCShader;
    // Released through their virtual destructors; their types are not
    // recovered yet.
    void* mUnknown200[2];
    eastl::vector<RndTextureBase*> mProbeCaptureDownsampleTextures;
    eastl::vector<RndTextureBase*> mProbeCaptureHelperTextures;
    // Released through their virtual destructors; their types are not
    // recovered yet.
    void* mUnknown280[3];
};

static_assert(offsetof(RndLightGlobals, mSpotlightMesh) == 8);
static_assert(offsetof(RndLightGlobals, mSphereScale) == 16);
static_assert(offsetof(RndLightGlobals, mPrimaryGroupSize) == 24);
static_assert(offsetof(RndLightGlobals, mSecondaryGroupSize) == 32);
static_assert(offsetof(RndLightGlobals, mDirectionalShader) == 40);
static_assert(offsetof(RndLightGlobals, mTiledLightIdsCount) == 104);
static_assert(offsetof(RndLightGlobals, mTiledLightsCullShader) == 112);
static_assert(offsetof(RndLightGlobals, mErrorLightCookie) == 144);
static_assert(offsetof(RndLightGlobals, mInlineLightingTextures) == 152);
static_assert(offsetof(RndLightGlobals, mInlineLightingData) == 160);
static_assert(offsetof(RndLightGlobals, mSkinDiffusion) == 176);
static_assert(offsetof(RndLightGlobals, mTonemapShader) == 184);
static_assert(offsetof(RndLightGlobals, mUnknown200) == 200);
static_assert(
    offsetof(RndLightGlobals, mProbeCaptureDownsampleTextures) == 216);
static_assert(offsetof(RndLightGlobals, mProbeCaptureHelperTextures) == 248);
static_assert(offsetof(RndLightGlobals, mUnknown280) == 280);
static_assert(sizeof(RndLightGlobals) == 304);
