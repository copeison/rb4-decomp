#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Generates screen-space ambient occlusion from linear depth and the
// G-buffer normals.
class RndCShaderSSAOGen : public RndShaderCompute {
public:
    RndCShaderSSAOGen();             // 0x6D7130
    ~RndCShaderSSAOGen() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    unsigned long mLinearDepthBuffer;  // Resource indices.
    unsigned long mGBufferNormal;
    unsigned long mNoiseTex;
    unsigned long mSceneMask;
    unsigned long mOutputBuffer;
    unsigned long mCBufferSize;
    unsigned long mDimensions;         // Constant offsets.
    unsigned long mParams;
};

static_assert(offsetof(RndCShaderSSAOGen, mLinearDepthBuffer) == 288);
static_assert(offsetof(RndCShaderSSAOGen, mCBufferSize) == 328);
static_assert(sizeof(RndCShaderSSAOGen) == 352);
