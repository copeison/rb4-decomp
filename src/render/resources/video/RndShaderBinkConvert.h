#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Converts the Y, Cr, Cb, and optional alpha planes of a Bink frame to RGB.
// The vtable is at 0x192ABA8.
class RndShaderBinkConvert : public RndShader {
public:
    RndShaderBinkConvert();             // 0x5F4980
    ~RndShaderBinkConvert() override;   // 0x5F49E0, 0x5F49F0

    const char* _GetClassNameImpl() const override;   // 0x5F4E70
    const char* _GetShaderFilePath() const override;  // 0x5F4C90
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x5F4CA0

    // Field names are not in the reference map.
    RndShaderDefInfo mBinkAlphaPlane;
    unsigned long mYPlane;       // Resource indices.
    unsigned long mCRPlane;
    unsigned long mCBPlane;
    unsigned long mAPlane;
    unsigned long mYScale;       // Constant offsets and the buffer size.
    unsigned long mCRScale;
    unsigned long mCBScale;
    unsigned long mFullScale;
    unsigned long mFullOffset;
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndShaderBinkConvert, mBinkAlphaPlane) == 288);
static_assert(offsetof(RndShaderBinkConvert, mYPlane) == 312);
static_assert(offsetof(RndShaderBinkConvert, mYScale) == 344);
static_assert(offsetof(RndShaderBinkConvert, mCBufferSize) == 384);
static_assert(sizeof(RndShaderBinkConvert) == 392);
