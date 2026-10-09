#include "render/lighting/tiled/RndCShaderTiledLightsCull.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Threads in one tiled-lighting group. Name not in the reference map.
constexpr unsigned long kThreadsPerGroup = 64;

}  // namespace

// Reconstructed from eboot.elf at 0x4AB6D0.
Vector2i CalcTilesPerThreadGroup(unsigned long depthSlices) {
    const unsigned long tiles = kThreadsPerGroup / depthSlices;
    int counts[2] = {1, 1};
    if (depthSlices <= 32) {
        counts[0] = 2;
        if (tiles > static_cast<unsigned long>(2L * counts[1])) {
            unsigned long axis = 1;
            do {
                counts[axis] *= 2;
                axis ^= 1;
            } while (tiles > static_cast<unsigned long>(
                         static_cast<long>(counts[1]) * counts[0]));
        }
    }
    return Vector2i{counts[0], counts[1]};
}

// Reconstructed from eboot.elf at 0x6D8A20.
RndCShaderTiledLightsCull::RndCShaderTiledLightsCull()
    : mTilesPerThreadGroup{},
      mIsStereo{},
      mOnlySliceZero{},
      mPointLights(-1),
      mSpotLights(-1),
      mLightProbes(-1),
      mSliceZeroLightIds(-1),
      mTiledDepthRangeBuffer(-1),
      mRightEyeTiledDepthRangeBuffer(-1),
      mSceneMask(-1),
      mLightIdsCount(-1),
      mLightIds(-1),
      mScratchLightIds(-1),
      mLightIdRanges(-1),
      mDimensions(-1),
      mMonoDimensions(-1),
      mTileCounts(-1),
      mNumPosLights(-1),
      mNumNegLights(-1),
      mNumProbes(-1),
      mNumSliceZeroPosLights(-1),
      mNumSliceZeroNegLights(-1),
      mNumSliceZeroProbes(-1),
      mCamDir(-1),
      mCamNearFarParams(-1),
      mCamDepthRangeParams(-1),
      mFrustumCorners(-1),
      mFrustumWidths(-1),
      mCBufferSize(0) {}

RndCShaderTiledLightsCull::~RndCShaderTiledLightsCull() {}

const char* RndCShaderTiledLightsCull::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/TiledLightsCull.hlsl";
}

// Reconstructed from eboot.elf at 0x6D9910. Reads the device's settings
// without a null check.
void RndCShaderTiledLightsCull::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    const auto& settings = *TheRndDevice()->mSettings;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"), static_cast<int>(settings.mLightTileSize));
    fixedDefines.Add(
        Symbol("HX_TILE_DEPTH_SLICES"),
        static_cast<int>(settings.mLightTileDepthSlices));
    fixedDefines.Add(
        Symbol("HX_MAX_LIGHTS_PER_TILE"),
        static_cast<int>(settings.mMaxLightsPerTile));

    auto& compute = defines.GetDefines(kShaderProgramCompute);
    mIsStereo = compute.AddBool(Symbol("HX_IS_STEREO"));
    mOnlySliceZero = compute.AddBool(Symbol("HX_ONLY_SLICE_ZERO"));

    mTilesPerThreadGroup = CalcTilesPerThreadGroup(
        static_cast<unsigned long>(settings.mLightTileDepthSlices));
    fixedDefines.Add(
        Symbol("HX_TILES_PER_THREAD_GROUP_X"), mTilesPerThreadGroup.x);
    fixedDefines.Add(
        Symbol("HX_TILES_PER_THREAD_GROUP_Y"), mTilesPerThreadGroup.y);

    mPointLights = resources.AddComputeBufferCustomTyped(
        "gPointLights", "CSLightPoint", 0, kShaderProgramCompute);
    mSpotLights = resources.AddComputeBufferCustomTyped(
        "gSpotLights", "CSLightSpot", 0, kShaderProgramCompute);
    mLightProbes = resources.AddComputeBufferCustomTyped(
        "gLightProbes", "CSLightProbe", 0, kShaderProgramCompute);
    mSliceZeroLightIds = resources.AddComputeBuffer(
        "gSliceZeroLightIds", 0, kShaderNumericUInt, kShaderProgramCompute);
    mTiledDepthRangeBuffer = resources.AddTexture(
        "gTiledDepthRangeBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mRightEyeTiledDepthRangeBuffer = resources.AddTexture(
        "gRightEyeTiledDepthRangeBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSceneMask = resources.AddTexture(
        "gSceneMask",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mLightIdsCount = resources.AddComputeBufferWritable(
        "gLightIdsCount", 0, kShaderNumericUInt, kShaderProgramCompute);
    mLightIds = resources.AddComputeBufferCustomTypedWritable(
        "gLightIds", "Uint2As32", 0, kShaderProgramCompute);
    mScratchLightIds = resources.AddComputeBufferCustomTypedWritable(
        "gScratchLightIds", "Uint2As32", 0, kShaderProgramCompute);
    mLightIdRanges = resources.AddComputeBufferCustomTypedWritable(
        "gLightIdRanges", "CSLightIdRange", 0, kShaderProgramCompute);

    mDimensions = cbuffer.AddConstant(kShaderNumericFloat4, "gDimensions");
    mMonoDimensions =
        cbuffer.AddConstant(kShaderNumericFloat4, "gMonoDimensions");
    mTileCounts = cbuffer.AddConstant(kShaderNumericFloat4, "gTileCounts");
    mNumPosLights = cbuffer.AddConstant(kShaderNumericFloat2, "gNumPosLights");
    mNumNegLights = cbuffer.AddConstant(kShaderNumericFloat2, "gNumNegLights");
    mNumProbes = cbuffer.AddConstant(kShaderNumericFloat, "gNumProbes");
    mNumSliceZeroPosLights =
        cbuffer.AddConstant(kShaderNumericFloat2, "gNumSliceZeroPosLights");
    mNumSliceZeroNegLights =
        cbuffer.AddConstant(kShaderNumericFloat2, "gNumSliceZeroNegLights");
    mNumSliceZeroProbes =
        cbuffer.AddConstant(kShaderNumericFloat, "gNumSliceZeroProbes");
    mCamDir = cbuffer.AddConstant(kShaderNumericFloat3, "gCamDir");
    mCamNearFarParams =
        cbuffer.AddConstant(kShaderNumericFloat4, "gCamNearFarParams");
    mCamDepthRangeParams =
        cbuffer.AddConstant(kShaderNumericFloat2, "gCamDepthRangeParams");
    mFrustumCorners = cbuffer.AddConstantArray(
        kShaderNumericFloat3, 8, "gFrustumCorners");
    mFrustumWidths = cbuffer.AddConstant(kShaderNumericFloat4, "gFrustumWidths");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x6D9E00. Stereo culling never limits
// itself to depth slice zero.
bool RndCShaderTiledLightsCull::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (type != kShaderProgramCompute) {
        return true;
    }
    const bool mono = mIsStereo.GetValue(key) == 0;
    return mOnlySliceZero.GetValue(key) == 0 || mono;
}

const char* RndCShaderTiledLightsCull::_GetClassNameImpl() const {
    return "RndCShaderTiledLightsCull";
}
