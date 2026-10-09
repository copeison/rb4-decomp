#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndCameraContext;
class RndContext;
class RndTexture3D;
class RndTextureArray2D;
class RndTextureBase;
struct RndTiledLightsComputeBuffer;

// Computes the fog density and in-scattered light of each froxel of the
// volumetric scattering volume.
class RndCShaderVScatCalcDensityInscattering : public RndShaderCompute {
public:
    // What RndVolumetricScatteringCom::BeginAsyncUpdate passes. Name and
    // field names not in the reference map.
    struct Params {
        // The scene camera, or the combined stereo camera.
        const RndCameraContext* mCamera;
        // The volume's depth per slice, scaled by the fog density.
        float mDensityStep;
        float mLightIntensity;
        // The fog range, clamped to the camera.
        float mStartDist;
        float mEndDist;
        // The light manager's point, spot and directional light buffers.
        RndTiledLightsComputeBuffer* mLightBuffers;
        RndTextureArray2D* mSpotShadowMaps;
        RndBufferCollection* mBuffers;
        RndTexture3D* mOutput;
        RndTextureBase* mSkyTexture;
        float mHeightRangeBegin;
        float mHeightRangeEnd;
        // The baked height density waveform, or the default black 1D
        // texture.
        RndTextureBase* mHeightWaveform;
        bool mIsStereo;
    };

    RndCShaderVScatCalcDensityInscattering();             // 0x6D26D0
    ~RndCShaderVScatCalcDensityInscattering() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Fills the volume with each froxel's density and in-scattered light.
    // Not reconstructed. Name not in the reference map.
    void Dispatch(RndContext& context, Params& params);  // 0x6D2780

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
static_assert(
    offsetof(RndCShaderVScatCalcDensityInscattering::Params, mLightBuffers)
    == 0x18);
static_assert(
    offsetof(RndCShaderVScatCalcDensityInscattering::Params, mIsStereo)
    == 0x50);
static_assert(sizeof(RndCShaderVScatCalcDensityInscattering) == 504);
