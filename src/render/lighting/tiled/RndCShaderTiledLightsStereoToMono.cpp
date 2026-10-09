#include "render/lighting/tiled/RndCShaderTiledLightsStereoToMono.h"

#include "render/lighting/tiled/RndCShaderTiledLightsCull.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6DA550.
RndCShaderTiledLightsStereoToMono::RndCShaderTiledLightsStereoToMono()
    : mBothEyesLightIds(-1),
      mBothEyesLightIdRanges(-1),
      mSceneMask(-1),
      mLightIdsCount(-1),
      mCurEyeLightIds(-1),
      mCurEyeScratchLightIds(-1),
      mCurEyeLightIdRanges(-1),
      mWhichEye(-1),
      mBothEyesDimensions(-1),
      mOneEyeDimensions(-1),
      mFrustumWidths(-1),
      mCBufferSize(0) {}

RndCShaderTiledLightsStereoToMono::~RndCShaderTiledLightsStereoToMono() {}

const char* RndCShaderTiledLightsStereoToMono::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/TiledLightsStereoToMono.hlsl";
}

// Reconstructed from eboot.elf at 0x6DAC40. Reads the device's settings
// without a null check.
void RndCShaderTiledLightsStereoToMono::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup&,
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
    const auto tilesPerGroup = CalcTilesPerThreadGroup(
        static_cast<unsigned long>(settings.mLightTileDepthSlices));
    fixedDefines.Add(Symbol("HX_TILES_PER_THREAD_GROUP_X"), tilesPerGroup.x);
    fixedDefines.Add(Symbol("HX_TILES_PER_THREAD_GROUP_Y"), tilesPerGroup.y);

    mBothEyesLightIds = resources.AddComputeBufferCustomTyped(
        "gBothEyesLightIds", "Uint2As32", 0, kShaderProgramCompute);
    mBothEyesLightIdRanges = resources.AddComputeBufferCustomTyped(
        "gBothEyesLightIdRanges", "CSLightIdRange", 0, kShaderProgramCompute);
    mSceneMask = resources.AddTexture(
        "gSceneMask",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mLightIdsCount = resources.AddComputeBufferWritable(
        "gLightIdsCount", 0, kShaderNumericUInt, kShaderProgramCompute);
    mCurEyeLightIds = resources.AddComputeBufferCustomTypedWritable(
        "gCurEyeLightIds", "Uint2As32", 0, kShaderProgramCompute);
    mCurEyeScratchLightIds = resources.AddComputeBufferCustomTypedWritable(
        "gCurEyeScratchLightIds", "Uint2As32", 0, kShaderProgramCompute);
    mCurEyeLightIdRanges = resources.AddComputeBufferCustomTypedWritable(
        "gCurEyeLightIdRanges", "CSLightIdRange", 0, kShaderProgramCompute);

    mWhichEye = cbuffer.AddConstant(kShaderNumericFloat, "gWhichEye");
    mBothEyesDimensions =
        cbuffer.AddConstant(kShaderNumericFloat4, "gBothEyesDimensions");
    mOneEyeDimensions =
        cbuffer.AddConstant(kShaderNumericFloat4, "gOneEyeDimensions");
    mFrustumWidths = cbuffer.AddConstant(kShaderNumericFloat4, "gFrustumWidths");
    mCBufferSize = cbuffer.mSize;
}

const char* RndCShaderTiledLightsStereoToMono::_GetClassNameImpl() const {
    return "RndCShaderTiledLightsStereoToMono";
}
