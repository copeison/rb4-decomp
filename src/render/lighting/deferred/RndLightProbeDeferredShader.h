#pragma once

#include <cstddef>

#include "render/lighting/deferred/RndLightDeferredShader.h"

class RndContext;
class RndTextureBase;

// Accumulates a light probe into the probe buffer, optionally blending two
// probe states and computing ambient occlusion. The vtable is at 0x1907580.
class RndLightProbeDeferredShader : public RndLightDeferredShader {
public:
    // Field names are not in the reference map.
    struct Params : RndLightDeferredShader::Params {
        // Diffuse and specular cube maps of the first and second state.
        RndTextureBase* mTextures[2][2];
        RndTextureBase* mAOTex;
    };

    RndLightProbeDeferredShader();             // 0x49DE70
    ~RndLightProbeDeferredShader() override;   // 0x49DED0, 0x49DEE0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x49E490
    const char* _GetShaderFilePath() const override; // slot 3 at 0x49E200
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x49E210
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // slot 7 at 0x49E420

    // Binds the probe cube maps, falling back to the zero cube, and the AO
    // texture. The states blend only when all four cube maps are present.
    void Select(RndContext& context, Params& params);  // 0x49DF00

    // Field names are not in the reference map.
    RndShaderDefInfo mBlendTextures;
    RndShaderDefInfo mCalcAO;
    unsigned long mFalloffParams;   // Constant offsets.
    unsigned long mBlendAmount;
    unsigned long mCamToLightXfm;
    // Diffuse and specular cube maps of the first and second state.
    unsigned long mTextures[2][2];  // Resource indices.
    unsigned long mAOTex;
};

static_assert(offsetof(RndLightProbeDeferredShader::Params, mTextures) == 8);
static_assert(offsetof(RndLightProbeDeferredShader::Params, mAOTex) == 40);
static_assert(sizeof(RndLightProbeDeferredShader::Params) == 48);
static_assert(offsetof(RndLightProbeDeferredShader, mBlendTextures) == 368);
static_assert(offsetof(RndLightProbeDeferredShader, mFalloffParams) == 408);
static_assert(offsetof(RndLightProbeDeferredShader, mTextures) == 432);
static_assert(sizeof(RndLightProbeDeferredShader) == 472);
