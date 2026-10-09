#pragma once

#include <cstddef>

#include "math/color/Color.h"
#include "render/shaders/RndShader.h"

class RndComputeBuffer;
class RndContext;
class RndTexture1D;
class RndTexture2D;
class RndTexture3D;
class RndTextureArray1D;
class RndTextureArray2D;
class RndTextureArrayCube;
class RndTextureBase;
class RndTextureCube;

// Draws one renderer buffer for RndBufferInspection: the texture mode picks
// how the buffer is sampled and the display mode how it is shown. The
// vtable is at 0x1937080. Field names are not in the reference map.
class RndBufferInspectionShader : public RndShader {
public:
    // The buffer to draw and how to show it. Absent textures fall back to
    // the device's error textures. Field names are not in the reference
    // map.
    struct Params {
        int mTexMode;      // An HX_BUFFER_TEXMODE value.
        int mDisplayMode;  // An HX_BUFFER_DISPMODE value.
        RndTexture1D* mTexture1D;
        RndTexture2D* mTexture2D;  // Also bound as the stencil buffer or unsampled.
        RndTexture3D* mTexture3D;
        RndTextureArray1D* mTextureArray1D;
        RndTextureArray2D* mTextureArray2D;
        RndTextureCube* mTextureCube;
        RndTextureArrayCube* mTextureArrayCube;
        RndTextureBase* mTexture2DRTSliced;
        RndTextureBase* mTextureArray2DRTSliced;  // Optional.
        // The 2D compute buffer, or the light ID ranges in the tiled
        // lighting modes.
        RndComputeBuffer* mComputeBuffer;
        RndComputeBuffer* mLightIds;
        int mAuxDims[2];  // Replaced by the compute buffer's square side.
        unsigned long mArrayElement;
        int mArrayParamZ;  // The third gTextureArrayParams value.
        Hmx::Color mTint;
        bool mDiscardPadPixels;
        float mDepthFrac;
        int mLightTileCounts[2];
        unsigned int mStencilParams[2];
    };

    RndBufferInspectionShader();             // 0x6B5FA0
    ~RndBufferInspectionShader() override;   // 0x6B6020, 0x6B6030

    const char* _GetClassNameImpl() const override;   // 0x6B7280
    const char* _GetShaderFilePath() const override;  // 0x6B6850
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x6B6860

    // Fills the constants, binds the buffer's resources (the device's
    // default textures when absent) and selects the texture mode's
    // permutation. RndBufferInspection's _DrawOneBuffer (0x6B43B0) calls
    // it through the inlined GetShader.
    void Select(RndContext& context, Params& params);  // 0x6B6050

    // The pixel define HX_BUFFER_TEXMODE.
    RndShaderDefInfo mTexMode;
    // Constant offsets and the buffer size.
    unsigned long mBufferDisplayMode;
    unsigned long mAuxDims;
    unsigned long mTextureArrayParams;
    unsigned long mTint;
    unsigned long mDiscardPadPixels;
    unsigned long mBufferInspectionDepthFrac;
    unsigned long mStencilParams;
    unsigned long mLightTileCounts;
    unsigned long mMaxOverdraw;  // Not set by the constructor.
    unsigned long mCBufferSize;
    // Resource indices.
    unsigned long mTexture1D;
    unsigned long mTexture2D;
    unsigned long mTexture3D;
    unsigned long mTextureArray1D;
    unsigned long mTextureArray2D;
    unsigned long mTextureCube;
    unsigned long mTextureArrayCube;
    unsigned long mTexture2DRTSliced;
    unsigned long mTextureArray2DRTSliced;
    unsigned long mStencilBuffer;
    unsigned long mTexture2DNoSampler;
    unsigned long mComputeBuffer2D;
    unsigned long mLightIds;
    unsigned long mLightIdRanges;
};

static_assert(offsetof(RndBufferInspectionShader::Params, mTexture1D) == 8);
static_assert(offsetof(RndBufferInspectionShader::Params, mTexture2DRTSliced) == 64);
static_assert(offsetof(RndBufferInspectionShader::Params, mComputeBuffer) == 80);
static_assert(offsetof(RndBufferInspectionShader::Params, mAuxDims) == 96);
static_assert(offsetof(RndBufferInspectionShader::Params, mArrayElement) == 104);
static_assert(offsetof(RndBufferInspectionShader::Params, mArrayParamZ) == 112);
static_assert(offsetof(RndBufferInspectionShader::Params, mTint) == 116);
static_assert(offsetof(RndBufferInspectionShader::Params, mDiscardPadPixels) == 132);
static_assert(offsetof(RndBufferInspectionShader::Params, mDepthFrac) == 136);
static_assert(offsetof(RndBufferInspectionShader::Params, mLightTileCounts) == 140);
static_assert(offsetof(RndBufferInspectionShader::Params, mStencilParams) == 148);
static_assert(sizeof(RndBufferInspectionShader::Params) == 160);

static_assert(offsetof(RndBufferInspectionShader, mTexMode) == 288);
static_assert(offsetof(RndBufferInspectionShader, mBufferDisplayMode) == 312);
static_assert(offsetof(RndBufferInspectionShader, mMaxOverdraw) == 376);
static_assert(offsetof(RndBufferInspectionShader, mCBufferSize) == 384);
static_assert(offsetof(RndBufferInspectionShader, mTexture1D) == 392);
static_assert(offsetof(RndBufferInspectionShader, mTexture2DNoSampler) == 472);
static_assert(offsetof(RndBufferInspectionShader, mLightIdRanges) == 496);
static_assert(sizeof(RndBufferInspectionShader) == 504);
