#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Classifies the tiles of a source buffer for the signed-distance pass. Not in
// the reference map; the name is the binary's class-name string.
class RndCShaderSignedDistanceClassify : public RndShaderCompute {
public:
    RndCShaderSignedDistanceClassify();             // 0x62EAF0
    ~RndCShaderSignedDistanceClassify() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    unsigned long mSrcBuffer;                 // Resource indices.
    unsigned long mDstBuffer;
    unsigned long mDstClassificationBuffer;
    unsigned long mCBufferSize;
    unsigned long mDimensions;                // Constant offsets.
    unsigned long mTileParams;
};

static_assert(offsetof(RndCShaderSignedDistanceClassify, mSrcBuffer) == 288);
static_assert(offsetof(RndCShaderSignedDistanceClassify, mCBufferSize) == 312);
static_assert(sizeof(RndCShaderSignedDistanceClassify) == 336);
