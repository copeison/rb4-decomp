#include "render/postprocessing/blur/RndCShaderBlurClassify.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x62E1C0. Every field, the buffer size
// included, starts at -1.
RndCShaderBlurClassify::RndCShaderBlurClassify()
    : mSrcBuffer(-1),
      mSrcClassificationBuffer(-1),
      mDstClassificationBuffer(-1),
      mCBufferSize(-1),
      mDimensions(-1),
      mTileParams(-1) {}

// Reconstructed from eboot.elf at 0x62E200.
RndCShaderBlurClassify::~RndCShaderBlurClassify() {}

// Reconstructed from eboot.elf at 0x62E630.
const char* RndCShaderBlurClassify::_GetClassNameImpl() const {
    return "RndCShaderBlurClassify";
}

// Reconstructed from eboot.elf at 0x62E530.
const char* RndCShaderBlurClassify::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/BlurClassify.hlsl";
}

// Reconstructed from eboot.elf at 0x62E540.
void RndCShaderBlurClassify::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSrcBuffer = resources.AddTexture(
        "gSrcBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSrcClassificationBuffer = resources.AddTexture(
        "gSrcClassificationBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDstClassificationBuffer = resources.AddTextureWritable(
        "gDstClassificationBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat2, "gDimensions");
    mTileParams = cbuffer.AddConstant(kShaderNumericFloat3, "gTileParams");
    mCBufferSize = cbuffer.mSize;
}
