#include "render/postprocessing/depth_of_field/RndCShaderDOFDiscBlur.h"

#include "render/core/settings/render_settings.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6F2B40.
RndCShaderDOFDiscBlur::RndCShaderDOFDiscBlur()
    : mSceneTex(-1),
      mNormalizedDepthTex(-1),
      mSceneMask(-1),
      mFunctionTable(-1),
      mOutputSceneTex(-1),
      mSprites(-1),
      mTextureSize(-1),
      mFalloffParams(-1),
      mBlurParams(-1),
      mOverbrightLuminance(-1),
      mCBufferSize(0) {}

RndCShaderDOFDiscBlur::~RndCShaderDOFDiscBlur() {}

const char* RndCShaderDOFDiscBlur::_GetClassNameImpl() const {
    return "RndCShaderDOFDiscBlur";
}

const char* RndCShaderDOFDiscBlur::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/DOFDiscBlur.hlsl";
}

void RndCShaderDOFDiscBlur::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSceneTex = resources.AddTexture(
        "gSceneTex",
        "gSceneTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mNormalizedDepthTex = resources.AddTexture(
        "gNormalizedDepthTex",
        "gNormalizedDepthTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSceneMask = resources.AddTexture(
        "gSceneMask",
        "gSceneMaskSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mFunctionTable = resources.AddTexture(
        "gFunctionTable",
        "gFunctionTableSampler",
        RndTextureBase::kTextureArray1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputSceneTex = resources.AddTextureWritable(
        "gOutputSceneTex",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    // Buffer usage 1.
    mSprites = resources.AddComputeBufferCustomTypedWritable(
        "gSprites", "CSBokehSprite", 1, kShaderProgramCompute);

    mTextureSize = cbuffer.AddConstant(kShaderNumericFloat4, "gTextureSize");
    mFalloffParams =
        cbuffer.AddConstant(kShaderNumericFloat3, "gFalloffParams");
    mBlurParams = cbuffer.AddConstant(kShaderNumericFloat2, "gBlurParams");
    mOverbrightLuminance =
        cbuffer.AddConstant(kShaderNumericFloat, "gOverbrightLuminance");
    mCBufferSize = cbuffer.mSize;

    // Without device settings the tile size defaults to 32.
    auto* device = TheRndDevice();
    const auto* settings = device != nullptr ? device->mSettings : nullptr;
    const int tileSize = settings != nullptr
        ? static_cast<int>(settings->light_tile_size)
        : 32;
    fixedDefines.Add(Symbol("HX_MAX_RADIUS"), tileSize / 2);
    fixedDefines.Add(Symbol("HX_TILE_SIZE"), tileSize);
}
