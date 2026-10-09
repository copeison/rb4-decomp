#include "render/lighting/fog/RndShaderFogDeferred.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x452CA0.
RndShaderFogDeferred::RndShaderFogDeferred()
    : mBT709ToBT2020{},
      mFalloffParams(-1),
      mSkyTex(-1),
      mLinearDepthMap(-1),
      mFunctionTable(-1) {
    _Register();
}

RndShaderFogDeferred::~RndShaderFogDeferred() {}

const char* RndShaderFogDeferred::_GetClassNameImpl() const {
    return "RndShaderFogDeferred";
}

const char* RndShaderFogDeferred::_GetShaderFilePath() const {
    return "../../system/data/shaders/FogDeferred.hlsl";
}

void RndShaderFogDeferred::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mBT709ToBT2020 = defines.GetDefines(kShaderProgramPixel)
                         .AddBool(Symbol("HX_BT709_TO_BT2020"));
    mFalloffParams = cbuffer.AddConstant(kShaderNumericFloat3, "gFalloffParams");
    mSkyTex = resources.AddTexture(
        "gSkyTex",
        "gSkyTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mLinearDepthMap = resources.AddTexture(
        "gLinearDepthMap",
        "gLinearDepthMapSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mFunctionTable = resources.AddTexture(
        "gFunctionTable",
        "gFunctionTableSampler",
        RndTextureBase::kTextureArray1D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}
