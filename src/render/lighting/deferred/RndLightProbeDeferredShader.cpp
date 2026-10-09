#include "render/lighting/deferred/RndLightProbeDeferredShader.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Name not in the reference map.
constexpr unsigned long kPixelKey = 3;

}  // namespace

// Reconstructed from eboot.elf at 0x49DE70. The constant offsets stay unset
// until _InitConfigImpl.
RndLightProbeDeferredShader::RndLightProbeDeferredShader()
    : mBlendTextures{},
      mCalcAO{},
      mTextures{
          {static_cast<unsigned long>(-1), static_cast<unsigned long>(-1)},
          {static_cast<unsigned long>(-1), static_cast<unsigned long>(-1)}},
      mAOTex(-1) {}

RndLightProbeDeferredShader::~RndLightProbeDeferredShader() {}

// Reconstructed from eboot.elf at 0x49DF00.
void RndLightProbeDeferredShader::Select(RndContext& context, Params& params) {
    bool blend = true;
    for (unsigned long i = 0; i < 2; ++i) {
        auto* first = params.mTextures[0][i];
        auto* second = params.mTextures[1][i];
        auto* zero = TheRndDevice()->mDefaults.GetTexture(
            RndTextureBase::kTextureCube, kDefaultTextureZero);
        if (first == nullptr) {
            first = zero;
            second = zero;
            blend = false;
        } else if (second == nullptr) {
            second = zero;
            blend = false;
        }
        first->Select(context, kShaderProgramPixel, mTextures[0][i], 0, 0);
        second->Select(context, kShaderProgramPixel, mTextures[1][i], 0, 0);
    }
    if (params.mAOTex != nullptr) {
        params.mAOTex->Select(context, kShaderProgramPixel, mAOTex, 0, 0);
    }
    RndShaderKeyGroup keys{};
    const auto key = mBlendTextures.SetValue(0, blend ? 1U : 0U);
    keys.mKeys[kPixelKey] =
        mCalcAO.SetValue(key, params.mAOTex != nullptr ? 1U : 0U);
    _SelectBase(context, params, keys);
}

const char* RndLightProbeDeferredShader::_GetShaderFilePath() const {
    return "../../system/data/shaders/LightProbeDeferred.hlsl";
}

// Reconstructed from eboot.elf at 0x49E210.
void RndLightProbeDeferredShader::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    RndLightDeferredShader::_InitConfigImpl(
        fixedDefines, defines, cbuffer, resources);
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mBlendTextures = pixel.AddBool(Symbol("HX_BLEND_TEXTURES"));
    mCalcAO = pixel.AddBool(Symbol("HX_CALC_AO"));

    mFalloffParams = cbuffer.AddConstant(kShaderNumericFloat3, "gFalloffParams");
    mBlendAmount = cbuffer.AddConstant(kShaderNumericFloat, "gBlendAmount");
    mCamToLightXfm =
        cbuffer.AddConstant(kShaderNumericFloat3x4, "gCamToLightXfm");

    mTextures[0][0] = resources.AddTexture(
        "gDiffuseCubetex0",
        "gDiffuseSampler0",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextures[1][0] = resources.AddTexture(
        "gDiffuseCubetex1",
        "gDiffuseSampler1",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextures[0][1] = resources.AddTexture(
        "gSpecularCubetex0",
        "gSpecularSampler0",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTextures[1][1] = resources.AddTexture(
        "gSpecularCubetex1",
        "gSpecularSampler1",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mAOTex = resources.AddTexture(
        "gAOTex",
        "gAOTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x49E420. Probes use only the first
// illumination type.
bool RndLightProbeDeferredShader::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (!RndLightDeferredShader::_UsesShaderKeyImpl(type, key)) {
        return false;
    }
    return type != kShaderProgramPixel || mIllumType.GetValue(key) == 0;
}

const char* RndLightProbeDeferredShader::_GetClassNameImpl() const {
    return "RndLightProbeDeferredShader";
}
