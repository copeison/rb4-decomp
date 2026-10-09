#pragma once

#include <cstddef>

#include "render/lighting/deferred/RndLightDeferredShader.h"

class RndContext;
class RndTextureBase;

// Accumulates a point light into the light buffer, optionally shadowed and
// cookied. The vtable is at 0x1907110.
class RndLightPointDeferredShader : public RndLightDeferredShader {
public:
    // Field names are not in the reference map.
    struct Params : RndLightDeferredShader::Params {
        RndTextureBase* mShadowMaps;
        RndTextureBase* mCookie;
    };

    RndLightPointDeferredShader();             // 0x496D10
    ~RndLightPointDeferredShader() override;   // 0x496D70, 0x496D80

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x497170
    const char* _GetShaderFilePath() const override; // slot 3 at 0x496F20
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x496F30

    // Binds the shadow maps and the cookie and selects the permutation.
    void Select(RndContext& context, Params& params);  // 0x496DA0

    // Field names are not in the reference map.
    RndShaderDefInfo mBT709ToBT2020;
    RndShaderDefInfo mCastsShadows;
    RndShaderDefInfo mCookie;
    unsigned long mLightColor;        // Constant offsets.
    unsigned long mLightWrapParams;
    unsigned long mBulbRadius;
    unsigned long mFalloffParams;
    unsigned long mShadowMapIndex;
    unsigned long mLightPosCamSpace;
    unsigned long mLightClipPlane;
    unsigned long mCamToLightXfm;
    unsigned long mShadowMaps;        // Resource indices.
    unsigned long mCookieTex;
};

static_assert(offsetof(RndLightPointDeferredShader::Params, mShadowMaps) == 8);
static_assert(sizeof(RndLightPointDeferredShader::Params) == 24);
static_assert(offsetof(RndLightPointDeferredShader, mBT709ToBT2020) == 368);
static_assert(offsetof(RndLightPointDeferredShader, mLightColor) == 432);
static_assert(offsetof(RndLightPointDeferredShader, mShadowMaps) == 496);
static_assert(sizeof(RndLightPointDeferredShader) == 512);
