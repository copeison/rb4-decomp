#include "render/lighting/ambient_occlusion/RndCShaderSSAOGen.h"

#include "render/core/settings/render_settings.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6D7130. The constructor leaves the
// buffer size unset; _InitConfigImpl assigns it.
RndCShaderSSAOGen::RndCShaderSSAOGen()
    : mLinearDepthBuffer(-1),
      mGBufferNormal(-1),
      mNoiseTex(-1),
      mSceneMask(-1),
      mOutputBuffer(-1),
      mDimensions(-1),
      mParams(-1) {}

RndCShaderSSAOGen::~RndCShaderSSAOGen() {}

const char* RndCShaderSSAOGen::_GetClassNameImpl() const {
    return "RndCShaderSSAOGen";
}

const char* RndCShaderSSAOGen::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/SSAOGen.hlsl";
}

// Reads the device's settings without a null check.
void RndCShaderSSAOGen::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mLinearDepthBuffer = resources.AddTexture(
        "gLinearDepthBuffer",
        "gLinearDepthBufferSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mGBufferNormal = resources.AddTexture(
        "gGBufferNormal",
        "gGBufferNormalSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mNoiseTex = resources.AddTexture(
        "gNoiseTex",
        "gNoiseTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSceneMask = resources.AddTexture(
        "gSceneMask",
        "gSceneMaskSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputBuffer = resources.AddTextureWritable(
        "gOutputBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat4, "gDimensions");
    mParams = cbuffer.AddConstant(kShaderNumericFloat4, "gParams");
    mCBufferSize = cbuffer.mSize;

    const auto& settings = *TheRndDevice()->mSettings;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"), static_cast<int>(settings.light_tile_size));
}
