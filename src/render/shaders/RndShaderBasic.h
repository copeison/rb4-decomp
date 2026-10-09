#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Flat-colored or textured geometry for debug and UI drawing. The vtable is
// at 0x192F018.
class RndShaderBasic : public RndShader {
public:
    RndShaderBasic();             // 0x6398D0
    ~RndShaderBasic() override;   // 0x639950, 0x639960

    const char* _GetClassNameImpl() const override;   // 0x639F00
    const char* _GetShaderFilePath() const override;  // 0x639BD0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x639BE0
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // 0x639E40
    bool _SupportsRTSlicing() const override;          // 0x639EF0

    // Field names are not in the reference map.
    RndShaderDefInfo mShadingMode;
    RndShaderDefInfo mTextureMode;
    RndShaderDefInfo mAlphaCut;
    RndShaderDefInfo mUseTexRedAsAlpha;
    unsigned long mColor;          // Constant offsets and the buffer size.
    unsigned long mCBufferSize;
    unsigned long mTexture2D;      // Resource indices.
    unsigned long mTexture2DRTSliced;
};

static_assert(offsetof(RndShaderBasic, mShadingMode) == 288);
static_assert(offsetof(RndShaderBasic, mColor) == 368);
static_assert(offsetof(RndShaderBasic, mTexture2DRTSliced) == 392);
