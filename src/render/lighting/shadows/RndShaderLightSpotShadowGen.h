#pragma once

#include <cstddef>

#include "render/lighting/deferred/RndLightDeferredShader.h"

class RndContext;
class RndTextureBase;

// Resolves a spotlight's shadow map into the deferred shadow buffer and
// classifies the screen tiles for softening. The vtable is at 0x1908128.
class RndShaderLightSpotShadowGen : public RndLightDeferredShader {
public:
    // Field names are not in the reference map.
    struct Params : RndLightDeferredShader::Params {
        bool mSoften;
        bool mDownsampleDepth2x;
        RndTextureBase* mShadowMap;
        RndTextureBase* mTiledClassificationBuffer;
    };

    // Leaves the classification-buffer index unset until _InitConfigImpl.
    RndShaderLightSpotShadowGen();             // 0x4ABED0
    ~RndShaderLightSpotShadowGen() override;   // 0x4ABF20, 0x4ABF30

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x4AC2E0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x4AC0A0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x4AC0B0

    // Binds the shadow map and the writable classification buffer and
    // selects the permutation with the first illumination type.
    void Select(RndContext& context, Params& params);  // 0x4ABF50

    // Field names are not in the reference map.
    RndShaderDefInfo mSoften;
    RndShaderDefInfo mDownsampleDepth2x;
    unsigned long mLightMeshParams0;   // Constant offsets.
    unsigned long mLightMeshParams1;
    unsigned long mShadowParams;
    unsigned long mShadowNearFarParams;
    unsigned long mShadowOffset;
    unsigned long mSoftShadowParams;
    unsigned long mLightPosCamSpace;
    unsigned long mLightClipPlane;
    unsigned long mCamToLightXfm;
    unsigned long mShadowMapIndex;
    unsigned long mShadowMap;          // Resource indices.
    unsigned long mTiledClassificationBuffer;
};

static_assert(offsetof(RndShaderLightSpotShadowGen::Params, mSoften) == 4);
static_assert(offsetof(RndShaderLightSpotShadowGen::Params, mShadowMap) == 8);
static_assert(sizeof(RndShaderLightSpotShadowGen::Params) == 24);
static_assert(offsetof(RndShaderLightSpotShadowGen, mSoften) == 368);
static_assert(offsetof(RndShaderLightSpotShadowGen, mLightMeshParams0) == 408);
static_assert(offsetof(RndShaderLightSpotShadowGen, mShadowMap) == 488);
static_assert(sizeof(RndShaderLightSpotShadowGen) == 504);
