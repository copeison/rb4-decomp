#include "render/lighting/volumetric/RndCShaderVScatCalcDensityInscattering.h"

#include <cstring>

#include "render/buffers/RndComputeBuffer.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/lighting/tiled/RndTiledLightsComputeBuffer.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTexture3D.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndTextureArray2D.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x6D3420; only the 3D width is used. The initializer also
// builds two Vector3 constants of the same header (0x1AB15D8, 0x1AB15E4),
// which are not modelled. Names not in the reference map.
[[maybe_unused]] int gCalcDensityGroupSizeUnused = -1;  // 0x1AB15CC
[[maybe_unused]] int gCalcDensityGroupSize2D = 8;       // 0x1AB15D0
int gCalcDensityGroupSize3D = 4;                        // 0x1AB15D4

// Names not in the reference map.
constexpr unsigned long kComputeKey = 4;
constexpr unsigned int kHdr10Output = 1;
// The height scale when the height range is empty.
constexpr float kDegenerateHeightScale = 10000.0F;

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

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

// Reconstructed from eboot.elf at 0x6D2750.
RndCShaderVScatCalcDensityInscattering::
    ~RndCShaderVScatCalcDensityInscattering() {}

// Reconstructed from eboot.elf at 0x6D2780. One thread per froxel; the
// volume's extent is a multiple of the 3D group width, so the divisions
// are exact.
void RndCShaderVScatCalcDensityInscattering::Dispatch(
    RndContext& context,
    Params& params) {
    RndDevice* const device = TheRndDevice();
    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] = mBT709ToBT2020.SetValue(
        0, device->mHdrOutputMode == kHdr10Output ? 1U : 0U);
    _SelectShaderCollection(context, keys);

    params.mOutput->Select(
        context,
        kShaderProgramCompute,
        mOutputTex,
        RndShaderResource::kSelectReadWrite,
        0);
    params.mSkyTexture->Select(context, kShaderProgramCompute, mSkyTex, 0, 0);
    device->mShaderMgr.mFunctionTable->Select(
        context, kShaderProgramCompute, mFunctionTable, 0, 0);
    params.mHeightWaveform->Select(
        context, kShaderProgramCompute, mHeightWaveform, 0, 0);
    if (params.mSpotShadowMaps != nullptr) {
        params.mSpotShadowMaps->Select(
            context, kShaderProgramCompute, mSpotShadowMapArray, 0, 0);
    }
    const RndTiledLightsComputeBuffer* const lights = params.mLightBuffers;
    lights[kTiledLightsPoint].mBuffer->Select(
        context, kShaderProgramCompute, mPointLights, 0, 0);
    lights[kTiledLightsSpot].mBuffer->Select(
        context, kShaderProgramCompute, mSpotLights, 0, 0);
    lights[kTiledLightsDirectional].mBuffer->Select(
        context, kShaderProgramCompute, mDirectionalLights, 0, 0);
    RndBufferCollection& buffers = *params.mBuffers;
    auto& frame = buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
    if (params.mIsStereo) {
        frame.mStereoTiledLightIds[0]->Select(
            context, kShaderProgramCompute, mLightIds, 0, 0);
        frame.mStereoTiledLightIdRanges->Select(
            context, kShaderProgramCompute, mLightIdRanges, 0, 0);
    } else {
        frame.mTiledLightIds[0]->Select(
            context, kShaderProgramCompute, mLightIds, 0, 0);
        frame.mTiledLightIdRanges->Select(
            context, kShaderProgramCompute, mLightIdRanges, 0, 0);
    }

    const RndCameraSettings& camera = params.mCamera->GetCameraSettings();
    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto member = [&cbuffer](unsigned long offset) {
        return static_cast<float*>(
            RndShaderDrawUtl::GetCBufferMember(cbuffer, offset));
    };
    // A 32-bit shader bool.
    auto* const isStereo = reinterpret_cast<unsigned char*>(member(mIsStereo));
    isStereo[0] = params.mIsStereo;
    isStereo[1] = 0;
    isStereo[2] = 0;
    isStereo[3] = 0;
    *member(mNumDirectionalLights) = static_cast<float>(
        lights[kTiledLightsDirectional].mNumPosLights);
    *member(mFogDensity) = params.mDensityStep;
    *member(mVolumetricLightingIntensity) = params.mLightIntensity;

    // The fog range as fractions of the camera's clip range.
    const float invClipRange = 1.0F / (camera.mFarPlane - camera.mNearPlane);
    float* const depthFrac = member(mDepthFracOffsetParams);
    depthFrac[0] = (params.mEndDist - params.mStartDist) * invClipRange;
    depthFrac[1] = (params.mStartDist - camera.mNearPlane) * invClipRange;

    float* const volumeDim = member(mVolumeDim);
    const auto& volume = params.mOutput->mBaseDesc;
    volumeDim[0] = static_cast<float>(static_cast<int>(volume.mWidth));
    volumeDim[1] = static_cast<float>(static_cast<int>(volume.mHeight));
    volumeDim[2] = static_cast<float>(static_cast<int>(volume.mDepth));
    float* const skyDim = member(mSkyTexDim);
    const auto& sky = params.mSkyTexture->mBaseDesc;
    skyDim[0] = static_cast<float>(static_cast<int>(sky.mWidth));
    skyDim[1] = static_cast<float>(static_cast<int>(sky.mHeight));

    // Maps a height onto [0, 1] across the waveform's range.
    const float heightRange = params.mHeightRangeEnd - params.mHeightRangeBegin;
    const float heightScale = heightRange != 0.0F
        ? 1.0F / heightRange
        : kDegenerateHeightScale;
    float* const height = member(mHeightParams);
    height[0] = heightScale;
    height[1] = -(params.mHeightRangeBegin * heightScale);

    const Vector2i size = buffers.mSize;
    float* const dimensions = member(mDimensions);
    dimensions[0] = static_cast<float>(size.x);
    dimensions[1] = static_cast<float>(size.y);
    const int tileSize = static_cast<int>(device->mSettings->mLightTileSize);
    float* const tileCounts = member(mTileCounts);
    tileCounts[0] = static_cast<float>(CeilDiv(size.x, tileSize));
    tileCounts[1] = static_cast<float>(CeilDiv(size.y, tileSize));

    float* const nearFar = member(mCamNearFarParams);
    nearFar[0] = camera.mNearPlane;
    nearFar[1] = camera.mFarPlane;
    nearFar[2] = camera.mFarPlane * camera.mNearPlane;
    nearFar[3] = camera.mFarPlane - camera.mNearPlane;

    // The world-to-camera transform as three rows of a 3x4 matrix.
    const Transform& inv = params.mCamera->mPrimaryView.mInvWorldXfm;
    float* const xfm = member(mCamWorldXfmInv);
    const Vector3* const axes[] = {&inv.m.x, &inv.m.y, &inv.m.z, &inv.v};
    for (int column = 0; column < 4; ++column) {
        xfm[column] = axes[column]->x;
        xfm[4 + column] = axes[column]->y;
        xfm[8 + column] = axes[column]->z;
    }

    // Each float3 array element takes a 16-byte register.
    float corners[8][4] = {};
    const Vector3* const frustum = params.mCamera->mFrustum.mCorners.mData;
    for (int i = 0; i < 8; ++i) {
        corners[i][0] = frustum[i].x;
        corners[i][1] = frustum[i].y;
        corners[i][2] = frustum[i].z;
    }
    std::memcpy(member(mFrustumCorners), corners, sizeof(corners));
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const auto groupSize =
        static_cast<unsigned int>(gCalcDensityGroupSize3D);
    context._DispatchComputeImpl(
        volume.mWidth / groupSize,
        volume.mHeight / groupSize,
        volume.mDepth / groupSize);
}

// Reconstructed from eboot.elf at 0x6D2FF0.
const char*
RndCShaderVScatCalcDensityInscattering::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/"
           "VScatCalcDensityInscattering.hlsl";
}

// Reconstructed from eboot.elf at 0x6D3000.
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

// Reconstructed from eboot.elf at 0x6D3410.
const char* RndCShaderVScatCalcDensityInscattering::_GetClassNameImpl() const {
    return "RndCShaderVScatCalcDensityInscattering";
}
