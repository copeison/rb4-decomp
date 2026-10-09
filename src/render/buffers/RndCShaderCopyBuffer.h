#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"
#include "render/shaders/RndShaderCBufferConfig.h"

class RndComputeBuffer;
class RndContext;
class RndTextureBase;

// Copies between uint or float4 buffers, 1D textures, and 2D textures.
class RndCShaderCopyBuffer : public RndShaderCompute {
public:
    // What to copy. A source texture is copied into the destination texture;
    // without one the source buffer is copied. Field names are not in the
    // reference map.
    struct Params {
        RndTextureBase* mSrcTexture;
        RndComputeBuffer* mSrcBuffer;
        RndTextureBase* mDestTexture;
        RndComputeBuffer* mDestBuffer;
        RndShaderNumericType mNumericType;  // kShaderNumericUInt or kShaderNumericFloat4.
    };

    RndCShaderCopyBuffer();             // 0x6F3550

    // Selects the permutation for the source's numeric and texture type,
    // binds the source and destination and dispatches one thread per
    // element.
    void Dispatch(RndContext& context, Params& params);  // 0x6F35E0
    ~RndCShaderCopyBuffer() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // 0x6F3D50

    // Field names are not in the reference map.
    RndShaderDefInfo mNumericType;
    RndShaderDefInfo mTextureType;
    unsigned long mSrcUintBuffer;    // Resource indices.
    unsigned long mSrcUintTex1D;
    unsigned long mSrcUintTex2D;
    unsigned long mSrcFloat4Buffer;
    unsigned long mSrcFloat4Tex1D;
    unsigned long mSrcFloat4Tex2D;
    unsigned long mDestUintBuffer;
    unsigned long mDestUintTex1D;
    unsigned long mDestUintTex2D;
    unsigned long mDestFloat4Buffer;
    unsigned long mDestFloat4Tex1D;
    unsigned long mDestFloat4Tex2D;
};

static_assert(offsetof(RndCShaderCopyBuffer, mNumericType) == 288);
static_assert(offsetof(RndCShaderCopyBuffer, mTextureType) == 308);
static_assert(offsetof(RndCShaderCopyBuffer, mSrcUintBuffer) == 328);
static_assert(sizeof(RndCShaderCopyBuffer) == 424);
