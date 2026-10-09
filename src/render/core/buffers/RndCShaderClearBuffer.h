#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Clears a uint or float4 buffer, 1D texture, or 2D texture to a color.
class RndCShaderClearBuffer : public RndShaderCompute {
public:
    RndCShaderClearBuffer();             // 0x637210
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
