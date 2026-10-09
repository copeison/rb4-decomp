#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Computes the fog density and in-scattered light of each froxel of the
// volumetric scattering volume.
class RndCShaderVScatCalcDensityInscattering : public RndShaderCompute {
public:
    RndCShaderVScatCalcDensityInscattering();             // 0x6D26D0
    ~RndCShaderVScatCalcDensityInscattering() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    RndShaderDefInfo mBT709ToBT2020;
    unsigned long mOutputTex;                    // Resource indices.
    unsigned long mSkyTex;
    unsigned long mFunctionTable;
    unsigned long mSpotShadowMapArray;
    unsigned long mHeightWaveform;
    unsigned long mPointLights;
    unsigned long mSpotLights;
    unsigned long mDirectionalLights;
    unsigned long mLightIds;
    unsigned long mLightIdRanges;
    unsigned long mIsStereo;                     // Constant offsets.
    unsigned long mNumDirectionalLights;
    unsigned long mFogDensity;
    unsigned long mVolumetricLightingIntensity;
    unsigned long mDepthFracOffsetParams;
    unsigned long mDimensions;
    unsigned long mVolumeDim;
    unsigned long mSkyTexDim;
    unsigned long mTileCounts;
    unsigned long mHeightParams;
    unsigned long mCamNearFarParams;
    unsigned long mCamWorldXfmInv;
    unsigned long mFrustumCorners;
    unsigned long mCBufferSize;
};

static_assert(
    offsetof(RndCShaderVScatCalcDensityInscattering, mBT709ToBT2020) == 288);
static_assert(
    offsetof(RndCShaderVScatCalcDensityInscattering, mOutputTex) == 312);
static_assert(
    offsetof(RndCShaderVScatCalcDensityInscattering, mPointLights) == 352);
static_assert(
    offsetof(RndCShaderVScatCalcDensityInscattering, mIsStereo) == 392);
static_assert(
    offsetof(RndCShaderVScatCalcDensityInscattering, mFrustumCorners) == 488);
static_assert(
    offsetof(RndCShaderVScatCalcDensityInscattering, mCBufferSize) == 496);
static_assert(sizeof(RndCShaderVScatCalcDensityInscattering) == 504);
