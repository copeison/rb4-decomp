#include "render/lighting/deferred/RndLightSpotDeferredShader.h"

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

// Reconstructed from eboot.elf at 0x4A86F0. The constant offsets stay unset
// until _InitConfigImpl.
RndLightSpotDeferredShader::RndLightSpotDeferredShader()
    : mBT709ToBT2020{},
      mCastsShadows{},
      mCookie{},
      mShadowMaps(-1),
      mCookieTex(-1) {}

RndLightSpotDeferredShader::~RndLightSpotDeferredShader() {}

// Reconstructed from eboot.elf at 0x4A8780.
void RndLightSpotDeferredShader::Select(RndContext& context, Params& params) {
    if (params.mShadowMaps != nullptr) {
        params.mShadowMaps->Select(context, kShaderProgramPixel, mShadowMaps, 0, 0);
    }
    if (params.mCookie != nullptr) {
        params.mCookie->Select(context, kShaderProgramPixel, mCookieTex, 0, 0);
    }
    RndShaderKeyGroup keys{};
    auto key = mBT709ToBT2020.SetValue(
        0, TheRndDevice()->mHdrOutputMode == kHdr10Output ? 1U : 0U);
    key = mCastsShadows.SetValue(key, params.mShadowMaps != nullptr ? 1U : 0U);
    keys.mKeys[kPixelKey] =
        mCookie.SetValue(key, params.mCookie != nullptr ? 1U : 0U);
    _SelectBase(context, params, keys);
}

const char* RndLightSpotDeferredShader::_GetShaderFilePath() const {
    return "../../system/data/shaders/LightSpotDeferred.hlsl";
}

// Reconstructed from eboot.elf at 0x4A8910.
void RndLightSpotDeferredShader::_InitConfigImpl(
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

    mLightMeshParams0 =
        cbuffer.AddConstant(kShaderNumericFloat4, "gLightMeshParams0");
    mLightMeshParams1 =
        cbuffer.AddConstant(kShaderNumericFloat2, "gLightMeshParams1");
    mLightColor = cbuffer.AddConstant(kShaderNumericFloat3, "gLightColor");
    mLightWrapParams =
        cbuffer.AddConstant(kShaderNumericFloat2, "gLightWrapParams");
    mBulbRadius = cbuffer.AddConstant(kShaderNumericFloat, "gBulbRadius");
    mFalloffParams = cbuffer.AddConstant(kShaderNumericFloat4, "gFalloffParams");
    mAngleFalloffParams =
        cbuffer.AddConstant(kShaderNumericFloat3, "gAngleFalloffParams");
    mCookieParams = cbuffer.AddConstant(kShaderNumericFloat4, "gCookieParams");
    mShadowMapIndex =
        cbuffer.AddConstant(kShaderNumericFloat, "gShadowMapIndex");
    mLightPosCamSpace =
        cbuffer.AddConstant(kShaderNumericFloat3, "gLightPosCamSpace");
    mLightClipPlane =
        cbuffer.AddConstant(kShaderNumericFloat4, "gLightClipPlane");
    mCamToLightXfm =
        cbuffer.AddConstant(kShaderNumericFloat3x4, "gCamToLightXfm");

    mCookieTex = resources.AddTexture(
        "gCookie",
        "gCookieSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mShadowMaps = resources.AddTexture(
        "gShadowMaps",
        "gShadowMapSampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

const char* RndLightSpotDeferredShader::_GetClassNameImpl() const {
    return "RndLightSpotDeferredShader";
}
