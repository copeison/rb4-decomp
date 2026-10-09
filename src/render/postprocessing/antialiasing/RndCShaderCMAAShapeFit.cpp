#include "render/postprocessing/antialiasing/RndCShaderCMAAShapeFit.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x451030.
RndCShaderCMAAShapeFit::RndCShaderCMAAShapeFit()
    : mInputEdges(-1),
      mOutputColor(-1),
      mOutputEdges(-1),
      mTargetSizeInv(-1),
      mCBufferSize(0) {}

RndCShaderCMAAShapeFit::~RndCShaderCMAAShapeFit() {}

const char* RndCShaderCMAAShapeFit::_GetClassNameImpl() const {
    return "RndCShaderCMAAShapeFit";
}

const char* RndCShaderCMAAShapeFit::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CMAAShapeFit.hlsl";
}

void RndCShaderCMAAShapeFit::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mInputEdges = resources.AddTexture(
        "gInputEdges",
        "gInputEdgesSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputColor = resources.AddTextureWritable(
        "gOutputColor",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputEdges = resources.AddTextureWritable(
        "gOutputEdges",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTargetSizeInv =
        cbuffer.AddConstant(kShaderNumericFloat2, "gTargetSizeInv");
    mCBufferSize = cbuffer.mSize;
}
