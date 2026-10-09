#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Accumulates the in-scattered light of the froxel volume along each view
// ray.
class RndCShaderVScatAccumScattering : public RndShaderCompute {
public:
    RndCShaderVScatAccumScattering();             // 0x6D1E70
    ~RndCShaderVScatAccumScattering() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    RndShaderDefInfo mUseSceneMask;
    RndShaderDefInfo mIsStereo;
    RndShaderDefInfo mStereoEye;
    unsigned long mDensityInscatteringTex;  // Resource indices.
    unsigned long mTiledDepthRangeBuffer;
    unsigned long mSceneMask;
    unsigned long mOutputTex;
    unsigned long mSrcDimensions;           // Constant offsets and the buffer
    unsigned long mDstDimensions;           // size.
    unsigned long mScreenDimensions;
    unsigned long mDepthFracOffsetParams;
    unsigned long mFrustumParams;
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderVScatAccumScattering, mUseSceneMask) == 288);
static_assert(
    offsetof(RndCShaderVScatAccumScattering, mDensityInscatteringTex) == 352);
static_assert(offsetof(RndCShaderVScatAccumScattering, mSrcDimensions) == 384);
static_assert(offsetof(RndCShaderVScatAccumScattering, mCBufferSize) == 424);
static_assert(sizeof(RndCShaderVScatAccumScattering) == 432);
