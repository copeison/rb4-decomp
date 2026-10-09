#include "render/postprocessing/antialiasing/RndShaderFXAA.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6364F0.
RndShaderFXAA::RndShaderFXAA()
    : mTargetDimensionsRcp(-1), mCBufferSize(0), mSrcTex(-1) {}

// Reconstructed from eboot.elf at 0x636540.
RndShaderFXAA::~RndShaderFXAA() {}

// Reconstructed from eboot.elf at 0x636770.
const char* RndShaderFXAA::_GetClassNameImpl() const {
    return "RndShaderFXAA";
}

// Reconstructed from eboot.elf at 0x6366F0.
const char* RndShaderFXAA::_GetShaderFilePath() const {
    return "../../system/data/shaders/FXAA.hlsl";
}

// Reconstructed from eboot.elf at 0x636700.
void RndShaderFXAA::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSrcTex = resources.AddTexture(
        "gSrcTex",
        "gSrcTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTargetDimensionsRcp =
        cbuffer.AddConstant(kShaderNumericFloat2, "gTargetDimensionsRcp");
    mCBufferSize = cbuffer.mSize;
}
