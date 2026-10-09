#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Computes each light tile's depth range from the linear depth buffer. Not
// in the reference map; the name is the binary's class-name string.
class RndCShaderCalcDepthRange : public RndShaderCompute {
public:
    RndCShaderCalcDepthRange();             // 0x636DE0
    ~RndCShaderCalcDepthRange() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    unsigned long mLinearDepthBuffer;  // Resource indices.
    unsigned long mTiledDepthRangeBuffer;
    unsigned long mTileCounts;         // Constant offset and the buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderCalcDepthRange, mLinearDepthBuffer) == 288);
static_assert(offsetof(RndCShaderCalcDepthRange, mCBufferSize) == 312);
static_assert(sizeof(RndCShaderCalcDepthRange) == 320);
