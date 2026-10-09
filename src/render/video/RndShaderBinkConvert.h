#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Converts the Y, Cr, Cb, and optional alpha planes of a Bink frame to RGB.
// The vtable is at 0x192ABA8.
class RndShaderBinkConvert : public RndShader {
public:
    // 128-byte parameter block assembled by the frame conversion. Name and
    // field names are not in the reference map.
    struct Params {
        RndTextureBase* mYPlane;
        RndTextureBase* mCRPlane;
        RndTextureBase* mCBPlane;
        RndTextureBase* mAPlane;  // Optional.
        float mYScale[4];
        float mCRScale[4];
        float mCBScale[4];
        // Named after the plane order; the shader has no alpha-plane scale
        // constant, and nothing writes or reads this one.
        float mAScale[4];
        float mFullScale[4];
        float mFullOffset[4];
    };

    RndShaderBinkConvert();             // 0x5F4980
    ~RndShaderBinkConvert() override;   // 0x5F49E0, 0x5F49F0

    const char* _GetClassNameImpl() const override;   // 0x5F4E70
    const char* _GetShaderFilePath() const override;  // 0x5F4C90
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x5F4CA0

    // Binds the planes, uploads the conversion constants, and selects the
    // alpha-plane permutation. Name not in the reference map.
    void Select(RndContext& context, const Params& params);  // 0x5F4A10

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

static_assert(offsetof(RndShaderBinkConvert::Params, mYScale) == 0x20);
static_assert(offsetof(RndShaderBinkConvert::Params, mFullScale) == 0x60);
static_assert(offsetof(RndShaderBinkConvert::Params, mFullOffset) == 0x70);
static_assert(sizeof(RndShaderBinkConvert::Params) == 0x80);

static_assert(offsetof(RndShaderBinkConvert, mBinkAlphaPlane) == 288);
static_assert(offsetof(RndShaderBinkConvert, mYPlane) == 312);
static_assert(offsetof(RndShaderBinkConvert, mYScale) == 344);
static_assert(offsetof(RndShaderBinkConvert, mCBufferSize) == 384);
static_assert(sizeof(RndShaderBinkConvert) == 392);
