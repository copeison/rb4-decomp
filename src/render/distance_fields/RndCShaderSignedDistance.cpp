#include "render/distance_fields/RndCShaderSignedDistance.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x62E660. Every field, the buffer size
// included, starts at -1.
RndCShaderSignedDistance::RndCShaderSignedDistance()
    : mSrcBuffer(-1),
      mClassificationBuffer(-1),
      mDstBuffer(-1),
      mCBufferSize(-1),
      mDimensions(-1),
      mTileParams(-1),
      mDistanceParams(-1) {}

RndCShaderSignedDistance::~RndCShaderSignedDistance() {}

const char* RndCShaderSignedDistance::_GetClassNameImpl() const {
    return "RndCShaderSignedDistance";
}

const char* RndCShaderSignedDistance::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/SignedDistance.hlsl";
}

void RndCShaderSignedDistance::_InitConfigImpl(
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
    mClassificationBuffer = resources.AddTexture(
        "gClassificationBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDstBuffer = resources.AddTextureWritable(
        "gDstBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat2, "gDimensions");
    mTileParams = cbuffer.AddConstant(kShaderNumericFloat3, "gTileParams");
    mDistanceParams =
        cbuffer.AddConstant(kShaderNumericFloat2, "gDistanceParams");
    mCBufferSize = cbuffer.mSize;
}
