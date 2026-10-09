#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Compute pass that converts the depth buffer to linear depth.
class RndCShaderLinearizeDepth : public RndShaderCompute {
public:
    RndCShaderLinearizeDepth();             // 0x637C10
    ~RndCShaderLinearizeDepth() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    RndShaderDefInfo mIsOrtho;
    unsigned long mDepth;        // Resource indices.
    unsigned long mLinearDepth;
    unsigned long mDimensions;   // Constant offset and the buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderLinearizeDepth, mIsOrtho) == 288);
static_assert(offsetof(RndCShaderLinearizeDepth, mDepth) == 312);
static_assert(offsetof(RndCShaderLinearizeDepth, mCBufferSize) == 336);
static_assert(sizeof(RndCShaderLinearizeDepth) == 344);
