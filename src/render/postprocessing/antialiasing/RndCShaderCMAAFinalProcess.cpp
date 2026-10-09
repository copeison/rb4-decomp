#include "render/postprocessing/antialiasing/RndCShaderCMAAFinalProcess.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x450BF0.
RndCShaderCMAAFinalProcess::RndCShaderCMAAFinalProcess()
    : mInputColor(-1),
      mInputEdges(-1),
      mOutputBuffer(-1),
      mTargetSizeInv(-1),
      mCBufferSize(0) {}

RndCShaderCMAAFinalProcess::~RndCShaderCMAAFinalProcess() {}

const char* RndCShaderCMAAFinalProcess::_GetClassNameImpl() const {
    return "RndCShaderCMAAFinalProcess";
}

const char* RndCShaderCMAAFinalProcess::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CMAAFinalProcess.hlsl";
}

void RndCShaderCMAAFinalProcess::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mInputColor = resources.AddTexture(
        "gInputColor",
        "gInputColorSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mInputEdges = resources.AddTexture(
        "gInputEdges",
        "gInputEdgesSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputBuffer = resources.AddTextureWritable(
        "gOutputBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTargetSizeInv =
        cbuffer.AddConstant(kShaderNumericFloat2, "gTargetSizeInv");
    mCBufferSize = cbuffer.mSize;
}
