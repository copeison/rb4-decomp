#pragma once

#include <cstddef>

#include "render/lighting/deferred/RndLightDeferredShader.h"

class RndContext;
class RndTextureBase;

// Accumulates a spotlight into the light buffer, optionally shadowed and
// cookied. The vtable is at 0x1907DB0.
class RndLightSpotDeferredShader : public RndLightDeferredShader {
public:
    // Field names are not in the reference map.
    struct Params : RndLightDeferredShader::Params {
        RndTextureBase* mShadowMaps;
        RndTextureBase* mCookie;
    };

    RndLightSpotDeferredShader();             // 0x4A86F0
    ~RndLightSpotDeferredShader() override;   // 0x4A8750, 0x4A8760

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x4A8BC0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x4A8900
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x4A8910

    // Binds the shadow maps and the cookie and selects the permutation.
    void Select(RndContext& context, Params& params);  // 0x4A8780

    // Field names are not in the reference map.
    RndShaderDefInfo mBT709ToBT2020;
    RndShaderDefInfo mCastsShadows;
    RndShaderDefInfo mCookie;
    unsigned long mLightMeshParams0;  // Constant offsets.
    unsigned long mLightMeshParams1;
    unsigned long mLightColor;
    unsigned long mLightWrapParams;
    unsigned long mBulbRadius;
    unsigned long mFalloffParams;
    unsigned long mAngleFalloffParams;
    unsigned long mCookieParams;
    unsigned long mShadowMapIndex;
    unsigned long mLightPosCamSpace;
    unsigned long mLightClipPlane;
    unsigned long mCamToLightXfm;
    unsigned long mShadowMaps;        // Resource indices.
    unsigned long mCookieTex;
};

static_assert(offsetof(RndLightSpotDeferredShader::Params, mShadowMaps) == 8);
static_assert(sizeof(RndLightSpotDeferredShader::Params) == 24);
static_assert(offsetof(RndLightSpotDeferredShader, mBT709ToBT2020) == 368);
static_assert(offsetof(RndLightSpotDeferredShader, mLightMeshParams0) == 432);
static_assert(offsetof(RndLightSpotDeferredShader, mShadowMaps) == 528);
static_assert(sizeof(RndLightSpotDeferredShader) == 544);
