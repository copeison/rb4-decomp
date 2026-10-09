#include "render/depth/RndCShaderLinearizeDepth.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x637C10.
RndCShaderLinearizeDepth::RndCShaderLinearizeDepth()
    : mIsOrtho{},
      mDepth(-1),
      mLinearDepth(-1),
      mDimensions(-1),
      mCBufferSize(0) {}

RndCShaderLinearizeDepth::~RndCShaderLinearizeDepth() {}

const char* RndCShaderLinearizeDepth::_GetClassNameImpl() const {
    return "RndCShaderLinearizeDepth";
}

const char* RndCShaderLinearizeDepth::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/LinearizeDepthCompute.hlsl";
}

void RndCShaderLinearizeDepth::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    fixedDefines.Add(Symbol("HX_TILE_SIZE_X"), 2);
    fixedDefines.Add(Symbol("HX_TILE_SIZE_Y"), 2);

    mIsOrtho = defines.GetDefines(kShaderProgramCompute)
                   .AddBool(Symbol("HX_IS_ORTHO"));

    mDepth = resources.AddTexture(
        "gDepth",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat);
    mLinearDepth = resources.AddTextureWritable(
        "gLinearDepth",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat2, "gDimensions");
    mCBufferSize = cbuffer.mSize;
}
