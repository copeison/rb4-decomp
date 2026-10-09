#include "render/core/debug/RndShaderDisplayTextureCube.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x63DFD0.
RndShaderDisplayTextureCube::RndShaderDisplayTextureCube()
    : mUseMipLevel{},
      mBlendTextures{},
      mUseTexArray{},
      mColor(-1),
      mMipLevel(-1),
      mBlendAmount(-1),
      mArrayIndices(-1),
      mCBufferSize(0),
      mTexture0(-1),
      mTexture1(-1),
      mTexArray(-1) {}

RndShaderDisplayTextureCube::~RndShaderDisplayTextureCube() {}

const char* RndShaderDisplayTextureCube::_GetClassNameImpl() const {
    return "RndShaderDisplayTextureCube";
}

const char* RndShaderDisplayTextureCube::_GetShaderFilePath() const {
    return "../../system/data/shaders/DisplayTextureCube.hlsl";
}

void RndShaderDisplayTextureCube::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mUseMipLevel = pixel.AddBool(Symbol("HX_USE_MIP_LEVEL"));
    mBlendTextures = pixel.AddBool(Symbol("HX_BLEND_TEXTURES"));
    mUseTexArray = pixel.AddBool(Symbol("HX_USE_TEXARRAY"));

    mColor = cbuffer.AddConstant(kShaderNumericFloat4, "gColor");
    mMipLevel = cbuffer.AddConstant(kShaderNumericFloat, "gMipLevel");
    mBlendAmount = cbuffer.AddConstant(kShaderNumericFloat, "gBlendAmount");
    mArrayIndices = cbuffer.AddConstant(kShaderNumericFloat2, "gArrayIndices");
    mCBufferSize = cbuffer.mSize;

    mTexture0 = resources.AddTexture(
        "gTexture0",
        "gTexSampler0",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexture1 = resources.AddTexture(
        "gTexture1",
        "gTexSampler1",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexArray = resources.AddTexture(
        "gTexArray",
        "gTexArraySampler",
        RndTextureBase::kTextureArrayCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}
