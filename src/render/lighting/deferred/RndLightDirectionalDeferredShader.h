#pragma once

#include <cstddef>

#include "render/lighting/deferred/RndLightDeferredShader.h"

class RndContext;
class RndTextureBase;

// Accumulates a directional light into the light buffer, optionally shadowed,
// cookied, and combined with one or two blended light probes. The vtable is
// at 0x1905910.
class RndLightDirectionalDeferredShader : public RndLightDeferredShader {
public:
    // Field names are not in the reference map.
    struct Params : RndLightDeferredShader::Params {
        RndTextureBase* mShadowMap;
        RndTextureBase* mCookie;
        bool mCombineWithProbe;
        bool mCombineWithProbeBlend;
        // Diffuse and specular cube maps of the first and second probe.
        RndTextureBase* mProbeTextures[2][2];
    };

    RndLightDirectionalDeferredShader();             // 0x477310
    ~RndLightDirectionalDeferredShader() override;   // 0x477390, 0x4773A0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x477BB0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x477750
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x477760
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // slot 7 at 0x477AF0

    // Binds the shadow map, the cookie and the probe cube maps, missing
    // probe textures falling back to the zero cube, and selects the
    // permutation.
    void Select(RndContext& context, Params& params);  // 0x4773C0

    // Field names are not in the reference map.
    RndShaderDefInfo mBT709ToBT2020;
    RndShaderDefInfo mCastsShadows;
    RndShaderDefInfo mCookie;
    RndShaderDefInfo mCombineWithProbe;
    RndShaderDefInfo mCombineWithProbeBlend;
    unsigned long mLightColor;        // Constant offsets.
    unsigned long mLightWrapParams;
    unsigned long mCookieScale;
    unsigned long mLightDirCamSpace;
    unsigned long mCamToLightXfm;
    unsigned long mProbeFalloffParams;
    unsigned long mProbeBlendAmount;
    unsigned long mCamToProbeXfm;
    unsigned long mShadowMap;         // Resource indices.
    unsigned long mCookieTex;
    // Diffuse and specular cube maps of the first and second probe.
    unsigned long mProbeTextures[2][2];
};

static_assert(
    offsetof(RndLightDirectionalDeferredShader::Params, mShadowMap) == 8);
static_assert(
    offsetof(RndLightDirectionalDeferredShader::Params, mCombineWithProbe) ==
    24);
static_assert(
    offsetof(RndLightDirectionalDeferredShader::Params, mProbeTextures) == 32);
static_assert(sizeof(RndLightDirectionalDeferredShader::Params) == 64);
static_assert(
    offsetof(RndLightDirectionalDeferredShader, mBT709ToBT2020) == 368);
static_assert(
    offsetof(RndLightDirectionalDeferredShader, mCombineWithProbeBlend) == 448);
static_assert(offsetof(RndLightDirectionalDeferredShader, mLightColor) == 472);
static_assert(offsetof(RndLightDirectionalDeferredShader, mShadowMap) == 536);
static_assert(
    offsetof(RndLightDirectionalDeferredShader, mProbeTextures) == 552);
static_assert(sizeof(RndLightDirectionalDeferredShader) == 584);
