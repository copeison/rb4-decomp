#include "render/debug/RndCShaderRenderTestCompute.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6F3E00.
RndCShaderRenderTestCompute::RndCShaderRenderTestCompute()
    : mWriteToBuffer{},
      mDstTexture(-1),
      mDstBuffer(-1),
      mDimensions(-1),
      mAnimParams(-1),
      mCBufferSize(0) {}

RndCShaderRenderTestCompute::~RndCShaderRenderTestCompute() {}

const char* RndCShaderRenderTestCompute::_GetClassNameImpl() const {
    return "RndCShaderRenderTestCompute";
}

const char* RndCShaderRenderTestCompute::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/RenderTestCompute.hlsl";
}

void RndCShaderRenderTestCompute::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mWriteToBuffer = defines.GetDefines(kShaderProgramCompute)
                         .AddBool(Symbol("HX_WRITE_TO_BUFFER"));
    mDstTexture = resources.AddTextureWritable(
        "gDstTexture",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDstBuffer = resources.AddComputeBufferWritable(
        "gDstBuffer", 0, kShaderNumericFloat4, kShaderProgramCompute);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat2, "gDimensions");
    mAnimParams = cbuffer.AddConstant(kShaderNumericFloat, "gAnimParams");
    mCBufferSize = cbuffer.mSize;
}
