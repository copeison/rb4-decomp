#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Blends the color along the fitted CMAA shapes.
// The Dispatch function is not reconstructed yet.
class RndCShaderCMAAFinalProcess : public RndShaderCompute {
public:
    RndCShaderCMAAFinalProcess();  // 0x450BF0
    ~RndCShaderCMAAFinalProcess() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    unsigned long mInputColor;     // Resource indices.
    unsigned long mInputEdges;
    unsigned long mOutputBuffer;
    unsigned long mTargetSizeInv;  // Constant offset, buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderCMAAFinalProcess, mInputColor) == 288);
static_assert(offsetof(RndCShaderCMAAFinalProcess, mCBufferSize) == 320);
static_assert(sizeof(RndCShaderCMAAFinalProcess) == 328);
