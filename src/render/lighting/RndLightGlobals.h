#pragma once

#include <cstddef>

#include "entity/core/EntityResource.h"
#include "entity/resources/Resource.h"
#include "render/textures/RndTexture2DResource.h"
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
class RndTexture2D;
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
    // Loads the inline lighting textures and the skin diffusion texture,
    // and takes the hair reflectance textures from the first's
    // RndTextureUtilityCom. The map's signature is LoadResources(); this
    // build reads RndInitParams::mInitRendering.
    void LoadResources(const RndInitParams& params);  // 0x47F8D0

    // The capture textures are found by width. Which getter reads which
    // vector follows the map's function order.
    RndTextureBase* GetProbeCaptureDownsampleTexture(int size);  // 0x47FD60
    RndTextureBase* GetProbeCaptureHelperTexture(int size);      // 0x47FDA0
    // The return type is not in the reference map.
    ResourcePath GetSkinDiffusionTexPath();  // 0x47FCD0
    // The texture of error_light_cookie.png, loaded on first use, or null
    // when it failed. Name not in the reference map. Not reconstructed.
    RndTexture2D* GetErrorLightCookie();  // 0x47FBF0

    void _InitBuffers();  // 0x47F500
    // Builds the spotlight volume and encodes per-vertex blend weights in
    // its vertex colors.
    void _InitLightSpotMesh();  // 0x47FDE0
    // Builds the point-light sphere and the spotlight volume.
    void _InitMeshes();  // 0x47F1C0
    // Creates and registers the lighting and tonemap shaders; the tiled
    // compute shaders only on platforms with async compute.
    void _InitShaders();  // 0x47F300

    // Field names are not in the reference map.
    RndMesh* mSphereMesh;     // "lighting_sphere"
    RndMesh* mSpotlightMesh;  // Created by _InitLightSpotMesh.
    // Derived by _InitMeshes from the sphere's tessellation: the factor by
    // which the circumscribed sphere mesh exceeds the unit sphere.
    float mSphereScale;
    // The spotlight volume's segments around its axis, and its cap
    // segments (the cap uses half of them).
    unsigned long mSpotlightSegments;
    unsigned long mSpotlightCapSegments;
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
    ResourcePtr<EntityResource> mInlineLightingTextures;
    // The hair reflectance textures of the inline lighting textures'
    // RndTextureUtilityCom, read by LoadResources.
    RndTextureBase* mHairReflectanceTextures[2];
    // skin_diffusion.bmp.
    ResourcePtr<RndTexture2DResource> mSkinDiffusion;
    RndTonemapShader* mTonemapShader;
    RndCShaderTonemap* mTonemapCShader;
    // The members below are released through their virtual destructors;
    // the light-probe capture (0x498A90) uses them. Their types are kept
    // opaque. Names not in the reference map.
    // The RndBufferCollection the probe capture renders into.
    void* mProbeCaptureBuffers;
    // The cube render target whose six faces the capture draws.
    void* mProbeCaptureTexture;
    eastl::vector<RndTextureBase*> mProbeCaptureDownsampleTextures;
    eastl::vector<RndTextureBase*> mProbeCaptureHelperTextures;
    // The filtered capture: the diffuse cube, then the specular cube whose
    // six mips are filtered from 128 down. The probe keeps copies of both.
    void* mProbeDiffuseCaptureTexture;
    void* mProbeSpecularCaptureTexture;
    // The RndCShaderFilterLightProbe compute shader that filters the
    // capture into the two cubes above.
    void* mFilterLightProbeCShader;
};

static_assert(offsetof(RndLightGlobals, mSpotlightMesh) == 8);
static_assert(offsetof(RndLightGlobals, mSphereScale) == 16);
static_assert(offsetof(RndLightGlobals, mSpotlightSegments) == 24);
static_assert(offsetof(RndLightGlobals, mSpotlightCapSegments) == 32);
static_assert(offsetof(RndLightGlobals, mDirectionalShader) == 40);
static_assert(offsetof(RndLightGlobals, mTiledLightIdsCount) == 104);
static_assert(offsetof(RndLightGlobals, mTiledLightsCullShader) == 112);
static_assert(offsetof(RndLightGlobals, mErrorLightCookie) == 144);
static_assert(offsetof(RndLightGlobals, mInlineLightingTextures) == 152);
static_assert(offsetof(RndLightGlobals, mHairReflectanceTextures) == 160);
static_assert(offsetof(RndLightGlobals, mSkinDiffusion) == 176);
static_assert(offsetof(RndLightGlobals, mTonemapShader) == 184);
static_assert(offsetof(RndLightGlobals, mProbeCaptureBuffers) == 200);
static_assert(
    offsetof(RndLightGlobals, mProbeCaptureDownsampleTextures) == 216);
static_assert(offsetof(RndLightGlobals, mProbeCaptureHelperTextures) == 248);
static_assert(offsetof(RndLightGlobals, mProbeDiffuseCaptureTexture) == 280);
static_assert(offsetof(RndLightGlobals, mFilterLightProbeCShader) == 296);
static_assert(sizeof(RndLightGlobals) == 304);
