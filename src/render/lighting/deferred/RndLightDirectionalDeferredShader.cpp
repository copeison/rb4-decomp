#include "render/lighting/deferred/RndLightDirectionalDeferredShader.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Names not in the reference map.
constexpr unsigned long kPixelKey = 3;
constexpr unsigned int kHdr10Output = 1;

}  // namespace

// Reconstructed from eboot.elf at 0x477310. The constant offsets stay unset
// until _InitConfigImpl.
RndLightDirectionalDeferredShader::RndLightDirectionalDeferredShader()
    : mBT709ToBT2020{},
      mCastsShadows{},
      mCookie{},
      mCombineWithProbe{},
      mCombineWithProbeBlend{},
      mShadowMap(-1),
      mCookieTex(-1),
      mProbeTextures{
          {static_cast<unsigned long>(-1), static_cast<unsigned long>(-1)},
          {static_cast<unsigned long>(-1), static_cast<unsigned long>(-1)}} {}

RndLightDirectionalDeferredShader::~RndLightDirectionalDeferredShader() {}

// Reconstructed from eboot.elf at 0x4773C0.
void RndLightDirectionalDeferredShader::Select(
    RndContext& context,
    Params& params) {
    if (params.mShadowMap != nullptr) {
        params.mShadowMap->Select(context, kShaderProgramPixel, mShadowMap, 0, 0);
    }
    if (params.mCookie != nullptr) {
        params.mCookie->Select(context, kShaderProgramPixel, mCookieTex, 0, 0);
    }
    if (params.mCombineWithProbe) {
        for (unsigned long i = 0; i < 2; ++i) {
            auto* first = params.mProbeTextures[0][i];
            auto* second = params.mProbeTextures[1][i];
            auto* zero = TheRndDevice()->mDefaults.GetTexture(
                RndTextureBase::kTextureCube, kDefaultTextureZero);
            if (first == nullptr) {
                first = zero;
                second = zero;
            } else if (second == nullptr) {
                second = zero;
            }
            first->Select(context, kShaderProgramPixel, mProbeTextures[0][i], 0, 0);
            second->Select(context, kShaderProgramPixel, mProbeTextures[1][i], 0, 0);
        }
    }

    const bool combine = params.mCombineWithProbe;
    RndShaderKeyGroup keys{};
    auto key = mBT709ToBT2020.SetValue(
        0, TheRndDevice()->mHdrOutputMode == kHdr10Output ? 1U : 0U);
    key = mCastsShadows.SetValue(key, params.mShadowMap != nullptr ? 1U : 0U);
    key = mCookie.SetValue(key, params.mCookie != nullptr ? 1U : 0U);
    key = mCombineWithProbe.SetValue(key, combine ? 1U : 0U);
    keys.mKeys[kPixelKey] = mCombineWithProbeBlend.SetValue(
        key, combine && params.mCombineWithProbeBlend ? 1U : 0U);
    _SelectBase(context, params, keys);
}

const char* RndLightDirectionalDeferredShader::_GetShaderFilePath() const {
    return "../../system/data/shaders/LightDirectionalDeferred.hlsl";
}

// Reconstructed from eboot.elf at 0x477760.
void RndLightDirectionalDeferredShader::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    RndLightDeferredShader::_InitConfigImpl(
        fixedDefines, defines, cbuffer, resources);
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mBT709ToBT2020 = pixel.AddBool(Symbol("HX_BT709_TO_BT2020"));
    mCastsShadows = pixel.AddBool(Symbol("HX_CASTS_SHADOWS"));
    mCookie = pixel.AddBool(Symbol("HX_COOKIE"));
    mCombineWithProbe = pixel.AddBool(Symbol("HX_COMBINE_WITH_PROBE"));
    mCombineWithProbeBlend =
        pixel.AddBool(Symbol("HX_COMBINE_WITH_PROBE_BLEND"));

    mLightColor = cbuffer.AddConstant(kShaderNumericFloat3, "gLightColor");
    mLightWrapParams =
        cbuffer.AddConstant(kShaderNumericFloat2, "gLightWrapParams");
    mCookieScale = cbuffer.AddConstant(kShaderNumericFloat2, "gCookieScale");
    mLightDirCamSpace =
        cbuffer.AddConstant(kShaderNumericFloat3, "gLightDirCamSpace");
    mCamToLightXfm =
        cbuffer.AddConstant(kShaderNumericFloat3x4, "gCamToLightXfm");
    mProbeFalloffParams =
        cbuffer.AddConstant(kShaderNumericFloat3, "gProbeFalloffParams");
    mProbeBlendAmount =
        cbuffer.AddConstant(kShaderNumericFloat, "gProbeBlendAmount");
    mCamToProbeXfm =
        cbuffer.AddConstant(kShaderNumericFloat3x4, "gCamToProbeXfm");

    mCookieTex = resources.AddTexture(
        "gCookie",
        "gCookieSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mShadowMap = resources.AddTexture(
        "gShadowMap",
        "gShadowMapSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mProbeTextures[0][0] = resources.AddTexture(
        "gCombineWithProbeDiffuseCubetex0",
        "gCombineWithProbeDiffuseSampler0",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mProbeTextures[1][0] = resources.AddTexture(
        "gCombineWithProbeDiffuseCubetex1",
        "gCombineWithProbeDiffuseSampler1",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mProbeTextures[0][1] = resources.AddTexture(
        "gCombineWithProbeSpecularCubetex0",
        "gCombineWithProbeSpecularSampler0",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mProbeTextures[1][1] = resources.AddTexture(
        "gCombineWithProbeSpecularCubetex1",
        "gCombineWithProbeSpecularSampler1",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x477AF0. Directional lights never use
// illumination type 3, are never shadowed in the pixel programs, and blend
// probes only when combining with one.
bool RndLightDirectionalDeferredShader::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (!RndLightDeferredShader::_UsesShaderKeyImpl(type, key)) {
        return false;
    }
    if (type == kShaderProgramPixel) {
        if (mIllumType.GetValue(key) == 3) {
            return false;
        }
        const auto combine = mCombineWithProbe.GetValue(key);
        const auto blend = mCombineWithProbeBlend.GetValue(key);
        if (combine == 0 && blend != 0) {
            return false;
        }
        if (mCastsShadows.GetValue(key) != 0) {
            return false;
        }
    }
    return true;
}

const char* RndLightDirectionalDeferredShader::_GetClassNameImpl() const {
    return "RndLightDirectionalDeferredShader";
}
