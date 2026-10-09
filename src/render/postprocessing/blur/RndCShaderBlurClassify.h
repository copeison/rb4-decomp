#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Compute pass that classifies blur tiles. The vtable is at 0x192E168.
class RndCShaderBlurClassify : public RndShaderCompute {
public:
    RndCShaderBlurClassify();             // 0x62E1C0
    ~RndCShaderBlurClassify() override;   // 0x62E200, 0x62E210

    const char* _GetClassNameImpl() const override;   // 0x62E630
    const char* _GetShaderFilePath() const override;  // 0x62E530
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x62E540

    // Field names are not in the reference map.
    unsigned long mSrcBuffer;            // Resource indices.
    unsigned long mSrcClassificationBuffer;
    unsigned long mDstClassificationBuffer;
    unsigned long mCBufferSize;          // Buffer size and constant offsets.
    unsigned long mDimensions;
    unsigned long mTileParams;
};

static_assert(offsetof(RndCShaderBlurClassify, mSrcBuffer) == 288);
static_assert(offsetof(RndCShaderBlurClassify, mCBufferSize) == 312);
static_assert(sizeof(RndCShaderBlurClassify) == 336);
