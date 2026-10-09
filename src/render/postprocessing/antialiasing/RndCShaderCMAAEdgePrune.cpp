#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgePrune.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x4508B0.
RndCShaderCMAAEdgePrune::RndCShaderCMAAEdgePrune()
    : mSourceEdges(-1),
      mOutputEdges(-1),
      mNonDominantEdgeThreshold(-1),
      mCBufferSize(0) {}

RndCShaderCMAAEdgePrune::~RndCShaderCMAAEdgePrune() {}

const char* RndCShaderCMAAEdgePrune::_GetClassNameImpl() const {
    return "RndCShaderCMAAEdgePrune";
}

const char* RndCShaderCMAAEdgePrune::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CMAAEdgePrune.hlsl";
}

void RndCShaderCMAAEdgePrune::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSourceEdges = resources.AddTexture(
        "gSourceEdges",
        "gSourceEdgesSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt4);
    mOutputEdges = resources.AddTextureWritable(
        "gOutputEdges",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mNonDominantEdgeThreshold = cbuffer.AddConstant(
        kShaderNumericFloat, "gNonDominantEdgeThreshold");
    mCBufferSize = cbuffer.mSize;
}
