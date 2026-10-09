#include "render/core/debug/RndShaderDisplayShadingMode.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x63DC90.
RndShaderDisplayShadingMode::RndShaderDisplayShadingMode()
    : mMaxOverdraw(-1), mCBufferSize(0), mSceneTex(-1) {}

RndShaderDisplayShadingMode::~RndShaderDisplayShadingMode() {}

const char* RndShaderDisplayShadingMode::_GetClassNameImpl() const {
    return "RndShaderDisplayShadingMode";
}

const char* RndShaderDisplayShadingMode::_GetShaderFilePath() const {
    return "../../system/data/shaders/DisplayShadingMode.hlsl";
}

void RndShaderDisplayShadingMode::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mMaxOverdraw = cbuffer.AddConstant(kShaderNumericFloat3, "gMaxOverdraw");
    mCBufferSize = cbuffer.mSize;
    mSceneTex = resources.AddTexture(
        "gSceneTex",
        "gSceneTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}
