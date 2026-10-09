#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgeDetect.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x450490.
RndCShaderCMAAEdgeDetect::RndCShaderCMAAEdgeDetect()
    : mSourceBuffer(-1),
      mOutputEdgesBuffer(-1),
      mOutputColorBuffer(-1),
      mEdgeThreshold(-1),
      mCBufferSize(0) {}

RndCShaderCMAAEdgeDetect::~RndCShaderCMAAEdgeDetect() {}

const char* RndCShaderCMAAEdgeDetect::_GetClassNameImpl() const {
    return "RndCShaderCMAAEdgeDetect";
}

const char* RndCShaderCMAAEdgeDetect::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CMAAEdgeDetect.hlsl";
}

void RndCShaderCMAAEdgeDetect::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSourceBuffer = resources.AddTexture(
        "gSourceBuffer",
        "gSourceBufferSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputEdgesBuffer = resources.AddTextureWritable(
        "gOutputEdgesBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt4);
    mOutputColorBuffer = resources.AddTextureWritable(
        "gOutputColorBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mEdgeThreshold = cbuffer.AddConstant(kShaderNumericFloat, "gEdgeThreshold");
    mCBufferSize = cbuffer.mSize;
}
