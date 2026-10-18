#pragma once

#include <cstddef>

#include "math/color/Color.h"
#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Displays a cube texture, optionally at a mip level, blended between two
// cubes, or from a cube array (render/RndShaderDisplayTextureCube.o,
// 0x63DFD0-0x63E5BF). The vtable is at 0x192F1C8.
class RndShaderDisplayTextureCube : public RndShader {
public:
    // Field names are not in the reference map.
    struct Params {
        // Without either texture the error cube stands in.
        RndTextureBase* mTexture0;
        // Blended in by mBlendAmount when set.
        RndTextureBase* mTexture1;
        // Takes precedence over the two cubes; it blends its two slices
        // when both indices are valid.
        RndTextureBase* mTexArray;
        unsigned long mArrayIndex0;  // -1 for none.
        unsigned long mArrayIndex1;  // -1 for none.
        float mBlendAmount;
        Hmx::Color mColor;
        bool mUseMipLevel;
        float mMipLevel;
    };

    RndShaderDisplayTextureCube();             // 0x63DFD0
    ~RndShaderDisplayTextureCube() override;  // 0x63E050, 0x63E060

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x63E590
    const char* _GetShaderFilePath() const override; // slot 3 at 0x63E370
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x63E380

    // Selects the shader and binds the cube textures for the pixel stage.
    void Select(RndContext& context, Params& params);  // 0x63E080

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

static_assert(
    offsetof(RndShaderDisplayTextureCube::Params, mBlendAmount) == 40);
static_assert(offsetof(RndShaderDisplayTextureCube::Params, mColor) == 44);
static_assert(
    offsetof(RndShaderDisplayTextureCube::Params, mUseMipLevel) == 60);
static_assert(offsetof(RndShaderDisplayTextureCube::Params, mMipLevel) == 64);

static_assert(offsetof(RndShaderDisplayTextureCube, mUseMipLevel) == 288);
static_assert(offsetof(RndShaderDisplayTextureCube, mColor) == 352);
static_assert(offsetof(RndShaderDisplayTextureCube, mCBufferSize) == 384);
static_assert(offsetof(RndShaderDisplayTextureCube, mTexArray) == 408);
static_assert(sizeof(RndShaderDisplayTextureCube) == 416);
