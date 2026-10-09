#include "render/distance_fields/RndCShaderSignedDistanceClassify.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x62EAF0. Every field, the buffer size
// included, starts at -1.
RndCShaderSignedDistanceClassify::RndCShaderSignedDistanceClassify()
    : mSrcBuffer(-1),
      mDstBuffer(-1),
      mDstClassificationBuffer(-1),
      mCBufferSize(-1),
      mDimensions(-1),
      mTileParams(-1) {}

RndCShaderSignedDistanceClassify::~RndCShaderSignedDistanceClassify() {}

const char* RndCShaderSignedDistanceClassify::_GetClassNameImpl() const {
    return "RndCShaderSignedDistanceClassify";
}

const char* RndCShaderSignedDistanceClassify::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/SignedDistanceClassify.hlsl";
}

void RndCShaderSignedDistanceClassify::_InitConfigImpl(
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
    mDstBuffer = resources.AddTextureWritable(
        "gDstBuffer",
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
