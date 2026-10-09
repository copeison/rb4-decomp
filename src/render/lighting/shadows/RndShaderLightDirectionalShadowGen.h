#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Resolves a directional light's cascaded shadow maps against the linear
// depth into a screen-space shadow buffer. Not in the reference map. The
// vtable is at 0x1908058.
class RndShaderLightDirectionalShadowGen : public RndShader {
public:
    // Field names are not in the reference map.
    struct Params {
        bool mSoften;
        unsigned long mNumCascades;
        RndTextureBase* mLinearDepth;
        RndTextureBase* mShadowMapArray;
    };

    RndShaderLightDirectionalShadowGen();             // 0x4AB780
    ~RndShaderLightDirectionalShadowGen() override;   // 0x4AB7E0, 0x4AB7F0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x4ABAF0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x4AB940
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x4AB950

    // Binds the linear depth and the shadow-map array and selects the
    // permutation. Name not in the reference map.
    void Select(RndContext& context, const Params& params);  // 0x4AB810

    // Field names are not in the reference map.
    RndShaderDefInfo mNumCascades;
    RndShaderDefInfo mSoften;
    unsigned long mShadowParams;          // Constant offsets.
    unsigned long mShadowInvOrthoYHeights;
    unsigned long mShadowNearFarParams;
    unsigned long mCamToShadowXfms;
    unsigned long mLinearDepth;           // Resource indices.
    unsigned long mShadowMapArray;
};

static_assert(
    offsetof(RndShaderLightDirectionalShadowGen::Params, mNumCascades) == 8);
static_assert(
    offsetof(RndShaderLightDirectionalShadowGen::Params, mLinearDepth) == 16);
static_assert(sizeof(RndShaderLightDirectionalShadowGen::Params) == 32);
static_assert(offsetof(RndShaderLightDirectionalShadowGen, mNumCascades) == 288);
static_assert(offsetof(RndShaderLightDirectionalShadowGen, mShadowParams) == 328);
static_assert(offsetof(RndShaderLightDirectionalShadowGen, mLinearDepth) == 360);
static_assert(sizeof(RndShaderLightDirectionalShadowGen) == 376);
