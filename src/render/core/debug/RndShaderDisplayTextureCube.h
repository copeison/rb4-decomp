#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Displays a cube texture, optionally at a mip level, blended between two
// cubes, or from a cube array.
class RndShaderDisplayTextureCube : public RndShader {
public:
    RndShaderDisplayTextureCube();             // 0x63DFD0
    ~RndShaderDisplayTextureCube() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    RndShaderDefInfo mUseMipLevel;
    RndShaderDefInfo mBlendTextures;
    RndShaderDefInfo mUseTexArray;
    unsigned long mColor;          // Constant offsets and the buffer size.
    unsigned long mMipLevel;
    unsigned long mBlendAmount;
    unsigned long mArrayIndices;
    unsigned long mCBufferSize;
    unsigned long mTexture0;       // Resource indices.
    unsigned long mTexture1;
    unsigned long mTexArray;
};

static_assert(offsetof(RndShaderDisplayTextureCube, mUseMipLevel) == 288);
static_assert(offsetof(RndShaderDisplayTextureCube, mColor) == 352);
static_assert(offsetof(RndShaderDisplayTextureCube, mCBufferSize) == 384);
static_assert(offsetof(RndShaderDisplayTextureCube, mTexArray) == 408);
static_assert(sizeof(RndShaderDisplayTextureCube) == 416);
