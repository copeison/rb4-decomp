#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"
#include "render/shaders/RndShaderCBufferConfig.h"

class RndComputeBuffer;
class RndContext;
class RndTextureBase;

// Clears a uint or float4 buffer, 1D texture, or 2D texture to a color.
class RndCShaderClearBuffer : public RndShaderCompute {
public:
    // What to clear: a texture, or a buffer when there is no texture, to a
    // value of the numeric type. Field names are not in the reference map.
    struct Params {
        RndTextureBase* mTexture;
        RndComputeBuffer* mBuffer;
        float mClearValue[4];
        RndShaderNumericType mNumericType;  // kShaderNumericUInt or kShaderNumericFloat4.
    };

    RndCShaderClearBuffer();             // 0x637210

    // Selects the permutation, commits the clear value and binds the target
    // read-write, returning its slot. Without a target only the slot is
    // returned, for callers that bind their own buffer. Inlined into
    // Dispatch. Name not in the reference map.
    unsigned long Select(RndContext& context, Params& params);  // 0x6372B0
    // Clears the target with one thread per element.
    void Dispatch(RndContext& context, Params& params);  // 0x6374F0
    ~RndCShaderClearBuffer() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // 0x637B60

    // Field names are not in the reference map.
    RndShaderDefInfo mNumericType;
    RndShaderDefInfo mTextureType;
    unsigned long mClearColor;     // Constant offset and the buffer size.
    unsigned long mCBufferSize;
    unsigned long mUintBuffer;     // Resource indices.
    unsigned long mUintTex1D;
    unsigned long mUintTex2D;
    unsigned long mFloat4Buffer;
    unsigned long mFloat4Tex1D;
    unsigned long mFloat4Tex2D;
};

static_assert(offsetof(RndCShaderClearBuffer, mNumericType) == 288);
static_assert(offsetof(RndCShaderClearBuffer, mTextureType) == 308);
static_assert(offsetof(RndCShaderClearBuffer, mClearColor) == 328);
static_assert(sizeof(RndCShaderClearBuffer) == 392);
