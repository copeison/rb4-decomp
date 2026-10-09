#include "render/lighting/volumetric/RndCShaderVScatDeferred.h"

#include "render/system/RndConfig.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6D3490. The constructor leaves the
// scene-mask index unset; _InitConfigImpl assigns it.
RndCShaderVScatDeferred::RndCShaderVScatDeferred()
    : mUseSceneMask{},
      mAccumScatteringTex(-1),
      mSrcLightAccumBuffer(-1),
      mLinearDepthTex(-1),
      mDstLightAccumBuffer(-1),
      mDimensions(-1),
      mProj(-1),
      mDepthFracOffsetParams(-1),
      mFogEndDistance(-1),
      mFogDensity(-1),
      mTexelSize(-1),
      mCBufferSize(0) {}

RndCShaderVScatDeferred::~RndCShaderVScatDeferred() {}

const char* RndCShaderVScatDeferred::_GetClassNameImpl() const {
    return "RndCShaderVScatDeferred";
}

const char* RndCShaderVScatDeferred::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/VScatDeferred.hlsl";
}

void RndCShaderVScatDeferred::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto* device = TheRndDevice();
    const auto* settings = device != nullptr ? device->mSettings : nullptr;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"),
        static_cast<int>(
            settings != nullptr ? settings->mLightTileSize
                                : RndConfig::kDefaultLightTileSize));
    mUseSceneMask = defines.GetDefines(kShaderProgramCompute)
                        .AddBool(Symbol("HX_USE_SCENE_MASK"));

    mAccumScatteringTex = resources.AddTexture(
        "gAccumScatteringTex",
        "gAccumScatteringTexSampler",
        RndTextureBase::kTexture3D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSrcLightAccumBuffer = resources.AddTexture(
        "gSrcLightAccumBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mLinearDepthTex = resources.AddTexture(
        "gLinearDepthTex",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSceneMask = resources.AddTexture(
        "gSceneMask",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDstLightAccumBuffer = resources.AddTextureWritable(
        "gDstLightAccumBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat4, "gDimensions");
    mProj = cbuffer.AddConstant(kShaderNumericFloat4x4, "gProj");
    mDepthFracOffsetParams =
        cbuffer.AddConstant(kShaderNumericFloat2, "gDepthFracOffsetParams");
    mFogEndDistance =
        cbuffer.AddConstant(kShaderNumericFloat, "gFogEndDistance");
    mFogDensity = cbuffer.AddConstant(kShaderNumericFloat, "gFogDensity");
    mTexelSize = cbuffer.AddConstant(kShaderNumericFloat3, "gTexelSize");
    mCBufferSize = cbuffer.mSize;
}
