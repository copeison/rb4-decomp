#include "render/core/buffers/RndCShaderClearBuffer.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x637210.
RndCShaderClearBuffer::RndCShaderClearBuffer()
    : mNumericType{},
      mTextureType{},
      mClearColor(-1),
      mCBufferSize(0),
      mUintBuffer(-1),
      mUintTex1D(-1),
      mUintTex2D(-1),
      mFloat4Buffer(-1),
      mFloat4Tex1D(-1),
      mFloat4Tex2D(-1) {}

RndCShaderClearBuffer::~RndCShaderClearBuffer() {}

const char* RndCShaderClearBuffer::_GetClassNameImpl() const {
    return "RndCShaderClearBuffer";
}

const char* RndCShaderClearBuffer::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/ClearBuffer.hlsl";
}

void RndCShaderClearBuffer::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto& compute = defines.GetDefines(kShaderProgramCompute);
    mNumericType = compute.Add(Symbol("HX_NUMERIC_TYPE"), 0, 17);
    mTextureType = compute.Add(Symbol("HX_TEXTURE_TYPE"), -1, 8);

    fixedDefines.Add(Symbol("HX_NUMERIC_TYPE_UINT"), kShaderNumericUInt);
    fixedDefines.Add(Symbol("HX_NUMERIC_TYPE_FLOAT4"), kShaderNumericFloat4);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_INVALID"), -1);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_1D"), RndTextureBase::kTexture1D);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_2D"), RndTextureBase::kTexture2D);

    mClearColor = cbuffer.AddConstant(kShaderNumericFloat4, "gClearColor");
    mCBufferSize = cbuffer.mSize;
    mUintBuffer = resources.AddComputeBufferWritable(
        "gUintBuffer", 0, kShaderNumericUInt, kShaderProgramCompute);
    mUintTex1D = resources.AddTextureWritable(
        "gUintTex1D",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mUintTex2D = resources.AddTextureWritable(
        "gUintTex2D",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mFloat4Buffer = resources.AddComputeBufferWritable(
        "gFloat4Buffer", 0, kShaderNumericFloat4, kShaderProgramCompute);
    mFloat4Tex1D = resources.AddTextureWritable(
        "gFloat4Tex1D",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mFloat4Tex2D = resources.AddTextureWritable(
        "gFloat4Tex2D",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x637B60. Compute permutations require a
// uint or float4 numeric type and an invalid, 1D, or 2D texture type.
bool RndCShaderClearBuffer::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (type != kShaderProgramCompute) {
        return true;
    }
    const auto numericType = mNumericType.GetValue(key);
    if (numericType != kShaderNumericFloat4 &&
        numericType != kShaderNumericUInt) {
        return false;
    }
    return mTextureType.GetValue(key) + 1U < 3U;
}
