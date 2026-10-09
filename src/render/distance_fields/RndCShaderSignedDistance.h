#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Signed-distance pass over a source buffer, guided by the classification
// buffer. Not in the reference map; the name is the binary's class-name
// string.
class RndCShaderSignedDistance : public RndShaderCompute {
public:
    RndCShaderSignedDistance();             // 0x62E660
    ~RndCShaderSignedDistance() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    unsigned long mSrcBuffer;              // Resource indices.
    unsigned long mClassificationBuffer;
    unsigned long mDstBuffer;
    unsigned long mCBufferSize;
    unsigned long mDimensions;             // Constant offsets.
    unsigned long mTileParams;
    unsigned long mDistanceParams;
};

static_assert(offsetof(RndCShaderSignedDistance, mSrcBuffer) == 288);
static_assert(offsetof(RndCShaderSignedDistance, mCBufferSize) == 312);
static_assert(sizeof(RndCShaderSignedDistance) == 344);
