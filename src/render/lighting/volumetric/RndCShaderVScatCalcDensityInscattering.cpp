#include "render/lighting/volumetric/RndCShaderVScatCalcDensityInscattering.h"

#include "render/system/RndConfig.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

namespace {

// The device's settings, or null when there is no device or no settings
// yet.
const RndConfig* CurrentSettings() {
    auto* device = TheRndDevice();
    return device != nullptr ? device->mSettings : nullptr;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6D26D0.
RndCShaderVScatCalcDensityInscattering::RndCShaderVScatCalcDensityInscattering()
    : mBT709ToBT2020{},
      mOutputTex(-1),
      mSkyTex(-1),
      mFunctionTable(-1),
      mSpotShadowMapArray(-1),
      mHeightWaveform(-1),
      mPointLights(-1),
      mSpotLights(-1),
      mDirectionalLights(-1),
      mLightIds(-1),
      mLightIdRanges(-1),
      mIsStereo(-1),
      mNumDirectionalLights(-1),
      mFogDensity(-1),
      mVolumetricLightingIntensity(-1),
      mDepthFracOffsetParams(-1),
      mDimensions(-1),
      mVolumeDim(-1),
      mSkyTexDim(-1),
      mTileCounts(-1),
      mHeightParams(-1),
      mCamNearFarParams(-1),
      mCamWorldXfmInv(-1),
      mFrustumCorners(-1),
      mCBufferSize(0) {}

RndCShaderVScatCalcDensityInscattering::
    ~RndCShaderVScatCalcDensityInscattering() {}

const char* RndCShaderVScatCalcDensityInscattering::_GetClassNameImpl() const {
    return "RndCShaderVScatCalcDensityInscattering";
}

const char*
RndCShaderVScatCalcDensityInscattering::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/"
           "VScatCalcDensityInscattering.hlsl";
}

void RndCShaderVScatCalcDensityInscattering::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    const auto* settings = CurrentSettings();
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"),
        static_cast<int>(
            settings != nullptr ? settings->mLightTileSize
                                : RndConfig::kDefaultLightTileSize));
    fixedDefines.Add(
        Symbol("HX_TILE_DEPTH_SLICES"),
        static_cast<int>(
            settings != nullptr ? settings->mLightTileDepthSlices
                                : RndConfig::kDefaultLightTileDepthSlices));
    fixedDefines.Add(Symbol("HX_SHADOW_CAST_CONTEXT_VOLUMETRIC"), 1);

    mBT709ToBT2020 = defines.GetDefines(kShaderProgramCompute)
                         .AddBool(Symbol("HX_BT709_TO_BT2020"));

    mOutputTex = resources.AddTextureWritable(
        "gOutputTex",
        RndTextureBase::kTexture3D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSkyTex = resources.AddTexture(
        "gSkyTex",
        "gSkyTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mFunctionTable = resources.AddTexture(
        "gFunctionTable",
        "gFunctionTableSampler",
        RndTextureBase::kTextureArray1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSpotShadowMapArray = resources.AddTexture(
        "gSpotShadowMapArray",
        "gSpotShadowMapArraySampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mHeightWaveform = resources.AddTexture(
        "gHeightWaveform",
        "gHeightWaveformSampler",
        RndTextureBase::kTexture1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mPointLights = resources.AddComputeBufferCustomTyped(
        "gPointLights", "CSLightPoint", 0, kShaderProgramCompute);
    mSpotLights = resources.AddComputeBufferCustomTyped(
        "gSpotLights", "CSLightSpot", 0, kShaderProgramCompute);
    mDirectionalLights = resources.AddComputeBufferCustomTyped(
        "gDirectionalLights", "CSLightDirectional", 0, kShaderProgramCompute);
    mLightIds = resources.AddComputeBufferCustomTyped(
        "gLightIds", "Uint2As32", 0, kShaderProgramCompute);
    mLightIdRanges = resources.AddComputeBufferCustomTyped(
        "gLightIdRanges", "CSLightIdRange", 0, kShaderProgramCompute);

    mIsStereo = cbuffer.AddConstant(kShaderNumericBool, "gIsStereo");
    mNumDirectionalLights =
        cbuffer.AddConstant(kShaderNumericFloat, "gNumDirectionalLights");
    mFogDensity = cbuffer.AddConstant(kShaderNumericFloat, "gFogDensity");
    mVolumetricLightingIntensity = cbuffer.AddConstant(
        kShaderNumericFloat, "gVolumetricLightingIntensity");
    mDepthFracOffsetParams =
        cbuffer.AddConstant(kShaderNumericFloat2, "gDepthFracOffsetParams");
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat2, "gDimensions");
    mVolumeDim = cbuffer.AddConstant(kShaderNumericFloat3, "gVolumeDim");
    mSkyTexDim = cbuffer.AddConstant(kShaderNumericFloat2, "gSkyTexDim");
    mTileCounts = cbuffer.AddConstant(kShaderNumericFloat2, "gTileCounts");
    mHeightParams = cbuffer.AddConstant(kShaderNumericFloat2, "gHeightParams");
    mCamNearFarParams =
        cbuffer.AddConstant(kShaderNumericFloat4, "gCamNearFarParams");
    mCamWorldXfmInv =
        cbuffer.AddConstant(kShaderNumericFloat3x4, "gCamWorldXfmInv");
    mFrustumCorners = cbuffer.AddConstantArray(
        kShaderNumericFloat3, 8, "gFrustumCorners");
    mCBufferSize = cbuffer.mSize;
}
