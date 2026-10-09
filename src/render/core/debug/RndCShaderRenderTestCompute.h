#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Compute render test that writes an animated pattern to a texture or a
// buffer. Not in the reference map.
class RndCShaderRenderTestCompute : public RndShaderCompute {
public:
    RndCShaderRenderTestCompute();             // 0x6F3E00
    ~RndCShaderRenderTestCompute() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    RndShaderDefInfo mWriteToBuffer;
    unsigned long mDstTexture;    // Resource indices.
    unsigned long mDstBuffer;
    unsigned long mDimensions;    // Constant offsets and the buffer size.
    unsigned long mAnimParams;
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderRenderTestCompute, mWriteToBuffer) == 288);
static_assert(offsetof(RndCShaderRenderTestCompute, mDstTexture) == 312);
static_assert(offsetof(RndCShaderRenderTestCompute, mCBufferSize) == 344);
static_assert(sizeof(RndCShaderRenderTestCompute) == 352);
