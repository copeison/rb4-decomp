#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndShaderResource;
class RndTextureBase;

// Separable Gaussian and depth-aware blur. The vtable is at 0x192EC08.
class RndShaderBlur : public RndShader {
public:
    RndShaderBlur();             // 0x634AE0
    ~RndShaderBlur() override;   // 0x634B80, 0x634B90

    const char* _GetClassNameImpl() const override;   // 0x635FB0
    const char* _GetShaderFilePath() const override;  // 0x635AF0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x635B00
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // 0x635F00

    // 48-byte parameter block for one blur pass, as the bloom stage fills
    // it (0x630C49). Name and field names are not in the reference map;
    // mBlurType, mTileSize and the two optional resources are named after
    // the defines and resources they select, which is weak evidence.
    struct Params {
        int mDirection;  // HX_BLUR_DIRECTION: 0 horizontal, 1 vertical.
        int mBlurType;
        RndTextureBase* mSource;
        RndTextureBase* mSceneMask;  // Optional.
        RndShaderResource* mClassificationBuffer;  // Optional.
        long mArraySlice;  // -1 for a plain texture.
        int mTileSize;     // Of the classification buffer.
        // The blur radius as a fraction of the source's width (horizontal)
        // or height (vertical); zero takes the default.
        float mRadius;
    };

    // Builds the Gaussian weights for the radius, binds the source and the
    // optional resources, and selects the programs.
    void Select(RndContext& context, const Params& params);  // 0x634BB0

    // Field names are not in the reference map.
    RndShaderDefInfo mIsTexArray;
    RndShaderDefInfo mNumBlurSamples;
    RndShaderDefInfo mBlurType;
    RndShaderDefInfo mBlurDirection;
    RndShaderDefInfo mUseSceneMask;
    RndShaderDefInfo mUseClassificationBuffer;
    unsigned long mArraySlice;           // Constant offsets and the buffer size.
    unsigned long mBlurSampleOffsetsWeights;
    unsigned long mMaxOffset;
    unsigned long mTileInfo;
    unsigned long mCBufferSize;
    unsigned long mTexture;              // Resource indices.
    unsigned long mTexArray;
    unsigned long mSceneMask;
    unsigned long mTiledClassificationBuffer;
};

static_assert(offsetof(RndShaderBlur::Params, mSource) == 8);
static_assert(offsetof(RndShaderBlur::Params, mArraySlice) == 0x20);
static_assert(offsetof(RndShaderBlur::Params, mRadius) == 0x2C);
static_assert(sizeof(RndShaderBlur::Params) == 48);

static_assert(offsetof(RndShaderBlur, mIsTexArray) == 288);
static_assert(offsetof(RndShaderBlur, mArraySlice) == 408);
static_assert(offsetof(RndShaderBlur, mTexture) == 448);
static_assert(sizeof(RndShaderBlur) == 480);
