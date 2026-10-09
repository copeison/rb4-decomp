#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndCameraContext;
class RndComputeBuffer;
class RndContext;
class RndTextureBase;
struct RndTiledLightsComputeBuffer;

// Shades every screen tile with the lights and probes the cull pass listed
// for it, accumulating into the light buffer. The vtable is at 0x1939070.
class RndCShaderTiledLightsApplication : public RndShaderCompute {
public:
    // Field names are not in the reference map. Bytes 16-56 are a copy of
    // the first 41 bytes of the caller's RndSceneDrawTarget
    // (_AccumTiledDeferredLight copies them at 0x485776); Dispatch reads
    // only mSceneContext from it.
    struct Params {
        const RndCameraContext* mCamera;
        RndBufferCollection* mBuffers;
        // RndSceneDrawTarget::mBuffersIndex and mLightAccumIndex.
        unsigned char mDrawParamsHeader[16];
        // Selects the light-accumulation buffer when the collection draws a
        // partial-framerate scene.
        unsigned long mSceneContext;
        // The rest of the copied draw parameters, then padding.
        unsigned char mDrawParamsRest[24];
        // Point, spot and directional lights.
        RndTiledLightsComputeBuffer* mLightBuffers;
        RndComputeBuffer* mLightProbes;
        // Missing cookie and probe textures fall back to the error textures.
        RndTextureBase* mCookies2D;
        RndTextureBase* mCookies2DRendered;
        RndTextureBase* mCookiesCube;
        RndTextureBase* mProbeDiffuseTextures;
        RndTextureBase* mProbeSpecularTextures;
        RndTextureBase* mAmbientOcclusion;
        RndTextureBase* mSrcLightAccumBuffer;
        float mProbeIntensityMult;
    };

    RndCShaderTiledLightsApplication();             // 0x6D7750
    ~RndCShaderTiledLightsApplication() override;   // 0x6D7810, 0x6D7820

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6D89F0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6D82E0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6D82F0
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // slot 7 at 0x6D8990

    // Binds the lights, the tile lists and the scene buffers under the
    // params' camera and dispatches one group per tile.
    void Dispatch(RndContext& context, Params& params);  // 0x6D7840

    // Field names are not in the reference map.
    RndShaderDefInfo mBT709ToBT2020;
    RndShaderDefInfo mInterpolateTiledLighting;
    RndShaderDefInfo mShadingMode;
    unsigned long mCookies2D;             // Resource indices.
    unsigned long mCookies2DRendered;
    unsigned long mCookiesCube;
    unsigned long mProbeDiffuseTextures;
    unsigned long mProbeSpecularTextures;
    unsigned long mAmbientOcclusion;
    unsigned long mFunctionTable;
    unsigned long mPointLights;
    unsigned long mSpotLights;
    unsigned long mDirectionalLights;
    unsigned long mLightProbes;
    unsigned long mLightIds;
    unsigned long mLightIdRanges;
    unsigned long mSrcLightAccumBuffer;
    unsigned long mGBuffers[3];
    unsigned long mLinearDepthBuffer;
    unsigned long mStencilBuffer;
    unsigned long mShadowMaps;
    unsigned long mDstLightAccumBuffer;
    unsigned long mLightInterpBuffer;
    unsigned long mCBufferSize;
    unsigned long mDimensions;            // Constant offsets.
    unsigned long mTileCounts;
    unsigned long mNumDirectionalLights;
    unsigned long mProbeIntensityMult;
};

static_assert(
    offsetof(RndCShaderTiledLightsApplication::Params, mSceneContext) == 32);
static_assert(
    offsetof(RndCShaderTiledLightsApplication::Params, mLightBuffers) == 64);
static_assert(
    offsetof(RndCShaderTiledLightsApplication::Params, mCookies2D) == 80);
static_assert(
    offsetof(RndCShaderTiledLightsApplication::Params, mProbeIntensityMult) ==
    136);
static_assert(
    offsetof(RndCShaderTiledLightsApplication, mInterpolateTiledLighting) ==
    308);
static_assert(offsetof(RndCShaderTiledLightsApplication, mCookies2D) == 352);
static_assert(offsetof(RndCShaderTiledLightsApplication, mGBuffers) == 464);
static_assert(offsetof(RndCShaderTiledLightsApplication, mCBufferSize) == 528);
static_assert(sizeof(RndCShaderTiledLightsApplication) == 568);
