#include "render/lighting/shadows/RndShaderLightDirectionalShadowGen.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Name not in the reference map.
constexpr unsigned long kPixelKey = 3;

}  // namespace

// Reconstructed from eboot.elf at 0x4AB780.
RndShaderLightDirectionalShadowGen::RndShaderLightDirectionalShadowGen()
    : mNumCascades{},
      mSoften{},
      mShadowParams(-1),
      mShadowInvOrthoYHeights(-1),
      mShadowNearFarParams(-1),
      mCamToShadowXfms(-1),
      mLinearDepth(-1),
      mShadowMapArray(-1) {}

RndShaderLightDirectionalShadowGen::~RndShaderLightDirectionalShadowGen() {}

// Reconstructed from eboot.elf at 0x4AB810.
void RndShaderLightDirectionalShadowGen::Select(
    RndContext& context,
    const Params& params) {
    params.mLinearDepth->Select(context, kShaderProgramPixel, mLinearDepth, 0, 0);
    params.mShadowMapArray->Select(
        context, kShaderProgramPixel, mShadowMapArray, 0, 0);
    RndShaderKeyGroup keys{};
    const auto key = mNumCascades.SetValue(
        0, static_cast<unsigned int>(params.mNumCascades));
    keys.mKeys[kPixelKey] = mSoften.SetValue(key, params.mSoften ? 1U : 0U);
    _SelectShaderCollection(context, keys);
}

const char* RndShaderLightDirectionalShadowGen::_GetShaderFilePath() const {
    return "../../system/data/shaders/LightDirectionalShadowGen.hlsl";
}

// Reconstructed from eboot.elf at 0x4AB950.
void RndShaderLightDirectionalShadowGen::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mNumCascades = pixel.Add(Symbol("HX_NUM_CASCADES"), 1, 5);
    mSoften = pixel.AddBool(Symbol("HX_SOFTEN"));
    mShadowParams = cbuffer.AddConstant(kShaderNumericFloat2, "gShadowParams");
    mShadowInvOrthoYHeights =
        cbuffer.AddConstant(kShaderNumericFloat4, "gShadowInvOrthoYHeights");
    mShadowNearFarParams = cbuffer.AddConstantArray(
        kShaderNumericFloat4, 4, "gShadowNearFarParams");
    mCamToShadowXfms = cbuffer.AddConstantArray(
        kShaderNumericFloat3x4, 4, "gCamToShadowXfms");
    mLinearDepth = resources.AddTexture(
        "gLinearDepth",
        "gLinearDepthSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mShadowMapArray = resources.AddTexture(
        "gShadowMapArray",
        "gShadowMapArraySampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

const char* RndShaderLightDirectionalShadowGen::_GetClassNameImpl() const {
    return "RndShaderLightDirectionalShadowGen";
}
