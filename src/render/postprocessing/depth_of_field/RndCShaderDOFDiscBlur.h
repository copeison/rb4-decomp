#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Depth-of-field disc blur: blurs the scene by the normalized depth and emits
// bokeh sprites for the overbright pixels. The Dispatch function is not
// reconstructed yet.
class RndCShaderDOFDiscBlur : public RndShaderCompute {
public:
    RndCShaderDOFDiscBlur();  // 0x6F2B40
    ~RndCShaderDOFDiscBlur() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    unsigned long mSceneTex;             // Resource indices.
    unsigned long mNormalizedDepthTex;
    unsigned long mSceneMask;
    unsigned long mFunctionTable;
    unsigned long mOutputSceneTex;
    unsigned long mSprites;
    unsigned long mTextureSize;          // Constant offsets and the buffer size.
    unsigned long mFalloffParams;
    unsigned long mBlurParams;
    unsigned long mOverbrightLuminance;
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderDOFDiscBlur, mSceneTex) == 288);
static_assert(offsetof(RndCShaderDOFDiscBlur, mTextureSize) == 336);
static_assert(offsetof(RndCShaderDOFDiscBlur, mCBufferSize) == 368);
static_assert(sizeof(RndCShaderDOFDiscBlur) == 376);
