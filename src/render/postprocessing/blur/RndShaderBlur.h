#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Separable Gaussian and depth-aware blur. The vtable is at 0x192EC08.
class RndShaderBlur : public RndShader {
public:
    RndShaderBlur();             // 0x634AE0
    ~RndShaderBlur() override;   // 0x634B80, 0x634B90

    const char* _GetClassNameImpl() const override;   // 0x635FB0
    const char* _GetShaderFilePath() const override;  // 0x635AF0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x635B00
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // 0x635F00

    // Field names are not in the reference map.
    RndShaderDefInfo mIsTexArray;
    RndShaderDefInfo mNumBlurSamples;
    RndShaderDefInfo mBlurType;
    RndShaderDefInfo mBlurDirection;
    RndShaderDefInfo mUseSceneMask;
    RndShaderDefInfo mUseClassificationBuffer;
    unsigned long mArraySlice;           // Constant offsets and the buffer size.
    unsigned long mBlurSampleOffsetsWeights;
    unsigned long mMaxOffset;
    unsigned long mTileInfo;
    unsigned long mCBufferSize;
    unsigned long mTexture;              // Resource indices.
    unsigned long mTexArray;
    unsigned long mSceneMask;
    unsigned long mTiledClassificationBuffer;
};

static_assert(offsetof(RndShaderBlur, mIsTexArray) == 288);
static_assert(offsetof(RndShaderBlur, mArraySlice) == 408);
static_assert(offsetof(RndShaderBlur, mTexture) == 448);
static_assert(sizeof(RndShaderBlur) == 480);
