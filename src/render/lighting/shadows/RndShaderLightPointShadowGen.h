#pragma once

#include <cstddef>

#include "render/lighting/deferred/RndLightDeferredShader.h"

class RndContext;
class RndTextureBase;

// Resolves a point light's cube shadow map into the deferred shadow buffer
// and classifies the screen tiles for softening. The vtable is at
// 0x19080C0.
class RndShaderLightPointShadowGen : public RndLightDeferredShader {
public:
    // Field names are not in the reference map.
    struct Params : RndLightDeferredShader::Params {
        bool mSoften;
        bool mDownsampleDepth2x;
        RndTextureBase* mShadowMap;
        RndTextureBase* mTiledClassificationBuffer;
    };

    RndShaderLightPointShadowGen();             // 0x4ABB20
    ~RndShaderLightPointShadowGen() override;   // 0x4ABB70, 0x4ABB80

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x4ABEA0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x4ABCF0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x4ABD00

    // Binds the shadow map and the writable classification buffer and
    // selects the permutation with the first illumination type.
    void Select(RndContext& context, Params& params);  // 0x4ABBA0

    // Field names are not in the reference map.
    RndShaderDefInfo mSoften;
    RndShaderDefInfo mDownsampleDepth2x;
    unsigned long mShadowCamParams;    // Constant offsets.
    unsigned long mShadowOffset;
    unsigned long mSoftShadowParams;
    unsigned long mLightPosCamSpace;
    unsigned long mCamToLightXfm;
    unsigned long mShadowMap;          // Resource indices.
    unsigned long mTiledClassificationBuffer;
};

static_assert(offsetof(RndShaderLightPointShadowGen::Params, mSoften) == 4);
static_assert(
    offsetof(RndShaderLightPointShadowGen::Params, mDownsampleDepth2x) == 5);
static_assert(offsetof(RndShaderLightPointShadowGen::Params, mShadowMap) == 8);
static_assert(sizeof(RndShaderLightPointShadowGen::Params) == 24);
static_assert(offsetof(RndShaderLightPointShadowGen, mSoften) == 368);
static_assert(offsetof(RndShaderLightPointShadowGen, mShadowCamParams) == 408);
static_assert(offsetof(RndShaderLightPointShadowGen, mShadowMap) == 448);
static_assert(sizeof(RndShaderLightPointShadowGen) == 464);
