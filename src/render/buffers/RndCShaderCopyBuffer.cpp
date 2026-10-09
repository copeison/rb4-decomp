#include "render/buffers/RndCShaderCopyBuffer.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6F3550.
RndCShaderCopyBuffer::RndCShaderCopyBuffer()
    : mNumericType{},
      mTextureType{},
      mSrcUintBuffer(-1),
      mSrcUintTex1D(-1),
      mSrcUintTex2D(-1),
      mSrcFloat4Buffer(-1),
      mSrcFloat4Tex1D(-1),
      mSrcFloat4Tex2D(-1),
      mDestUintBuffer(-1),
      mDestUintTex1D(-1),
      mDestUintTex2D(-1),
      mDestFloat4Buffer(-1),
      mDestFloat4Tex1D(-1),
      mDestFloat4Tex2D(-1) {}

RndCShaderCopyBuffer::~RndCShaderCopyBuffer() {}

const char* RndCShaderCopyBuffer::_GetClassNameImpl() const {
    return "RndCShaderCopyBuffer";
}

const char* RndCShaderCopyBuffer::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CopyBuffer.hlsl";
}

void RndCShaderCopyBuffer::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig& resources) {
    auto& compute = defines.GetDefines(kShaderProgramCompute);
    mNumericType = compute.Add(Symbol("HX_NUMERIC_TYPE"), 0, 17);
    mTextureType = compute.Add(Symbol("HX_TEXTURE_TYPE"), -1, 8);

    fixedDefines.Add(Symbol("HX_NUMERIC_TYPE_UINT"), kShaderNumericUInt);
    fixedDefines.Add(Symbol("HX_NUMERIC_TYPE_FLOAT4"), kShaderNumericFloat4);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_INVALID"), -1);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_1D"), RndTextureBase::kTexture1D);
    fixedDefines.Add(Symbol("HX_TEXTURE_TYPE_2D"), RndTextureBase::kTexture2D);

    mSrcUintBuffer = resources.AddComputeBuffer(
        "gSrcUintBuffer", 0, kShaderNumericUInt, kShaderProgramCompute);
    mSrcUintTex1D = resources.AddTexture(
        "gSrcUintTex1D",
        "gSrcUintTex1DSampler",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mSrcUintTex2D = resources.AddTexture(
        "gSrcUintTex2D",
        "gSrcUintTex2DSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mSrcFloat4Buffer = resources.AddComputeBuffer(
        "gSrcFloat4Buffer", 0, kShaderNumericFloat4, kShaderProgramCompute);
    mSrcFloat4Tex1D = resources.AddTexture(
        "gSrcFloat4Tex1D",
        "gSrcFloat4Tex1DSampler",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSrcFloat4Tex2D = resources.AddTexture(
        "gSrcFloat4Tex2D",
        "gSrcFloat4Tex2DSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDestUintBuffer = resources.AddComputeBufferWritable(
        "gDestUintBuffer", 0, kShaderNumericUInt, kShaderProgramCompute);
    mDestUintTex1D = resources.AddTextureWritable(
        "gDestUintTex1D",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mDestUintTex2D = resources.AddTextureWritable(
        "gDestUintTex2D",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt);
    mDestFloat4Buffer = resources.AddComputeBufferWritable(
        "gDestFloat4Buffer", 0, kShaderNumericFloat4, kShaderProgramCompute);
    mDestFloat4Tex1D = resources.AddTextureWritable(
        "gDestFloat4Tex1D",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDestFloat4Tex2D = resources.AddTextureWritable(
        "gDestFloat4Tex2D",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x6F3D50. Compute permutations require a
// uint or float4 numeric type and an invalid, 1D, or 2D texture type.
bool RndCShaderCopyBuffer::_UsesShaderKeyImpl(
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
