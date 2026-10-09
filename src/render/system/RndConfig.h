#pragma once

#include <cstddef>
#include <cstdint>

#include "math/vector/Vector2i.h"
#include "os/platform/PlatformMgr.h"
#include "render/debug/screenshot_capture.h"

// Render quality level read from the rnd config. Name not in the reference
// map.
enum class RndQualityLevel : std::uint32_t {
    kLow = 0,
    kMedium = 1,
    kHigh = 2,
    kInvalid = 0xFFFFFFFFu,
};

// The renderer's settings: the rnd config block, clamped to the current
// platform's capabilities, with the -resolution option applied. RndDevice
// allocates one in its constructor and owns it. Field names are not in the
// reference map.
class RndConfig {
public:
    // Default tile sizes, also used by shaders configured before the device
    // has settings. Names not in the reference map.
    static constexpr std::int64_t kDefaultLightTileSize = 32;
    static constexpr std::int64_t kDefaultLightTileDepthSlices = 8;

    RndConfig();  // 0x6BB470

    // The vsync mode, or zero when vsync is disabled. Name not in the
    // reference map.
    std::int32_t ActiveVSyncMode() const;

    Vector2i mContentResolution{1920, 1080};
    Vector2i mPcWindowResolution{1280, 720};
    bool mPcFullscreen = false;
    std::uint8_t mReserved17[3]{};
    std::int32_t mVSyncMode = 0;

    bool mUseLod = true;
    bool mUseGBufferVertexNormals = true;
    bool mUse64BitLightAccum = false;
    bool mUse40BitDepthStencil = true;
    bool mUseTiledLighting = false;
    std::uint8_t mReserved29[3]{};

    std::int64_t mLightTileSize = kDefaultLightTileSize;
    std::int64_t mLightTileDepthSlices = kDefaultLightTileDepthSlices;
    std::int64_t mVolumetricScatteringTileSize = 16;
    std::int64_t mMaxLightsPerTile = 256;
    std::int64_t mMaxPointLights = 256;
    std::int64_t mMaxSpotLights = 32;
    std::int64_t mMaxDirectionalLights = 16;
    std::int64_t mMaxLightProbes = 128;
    // Never read in this build, and not loaded from the rnd config; the
    // name is a guess from its place after the light-probe limit. Name not
    // in the reference map.
    bool mUseLightProbes = true;
    std::uint8_t mReserved97[7]{};

    std::int64_t mMaxPartialFramerateScenes = 0;
    std::int64_t mMaxShadowContribBuffers = 0;
    std::int64_t mShadowSoftenTileSize = 16;
    std::int64_t mMaskTileSize = 16;

    Vector2i mOutputResolution{1920, 1080};
    bool mResolutionOverridden = false;
    std::uint8_t mReserved145[3]{};
    RndQualityLevel mQualityLevel = RndQualityLevel::kMedium;
    bool mVSyncEnabled = true;

    bool mSceneMaskEnabled = true;
    bool mShadowsEnabled = true;
    bool mPostProcEnabled = true;
    bool mTonemappingEnabled = true;
    bool mVolumetricScatteringEnabled = true;
    std::uint8_t mReserved158[2]{};

    std::int64_t mFirstDrawnScene = -1;
    std::int64_t mLastDrawnScene = -1;
    bool mMultithreadedRenderingEnabled = true;
    bool mAsyncComputeEnabled = true;
    bool mAsyncCopyEnabled = true;
    bool mTiledLightInterpolationEnabled = true;
    bool mPartialFramerateEnabled = false;
    bool mStereoOptimizationsEnabled = true;
    std::uint8_t mReserved182[2]{};
    ScreenshotResolution mScreenshotResolution =
        ScreenshotResolution::kCurrent;
    std::uint8_t mReserved188[4]{};

    std::int64_t mMaxGeoOverdraw = 10;
    std::int64_t mMaxLightingOverdraw = 20;
    std::int64_t mMaxLightProbeOverdraw = 10;

    bool mGraphicsApiValidationEnabled = false;
    bool mBreakOnGraphicsWarning = false;
    bool mBreakOnGraphicsError = true;
    bool mGraphicsDebuggerEnabled = false;
    bool mGraphicsBarrierValidationEnabled = false;
    bool mPrintShaderCompilation = false;
    bool mPrintVerboseShaderCompilation = false;
    bool mOutputShaderIntermediates = false;
    bool mGenerateShaderDebugInfo = false;
    std::uint8_t mReserved225[7]{};
};

static_assert(offsetof(RndConfig, mVSyncMode) == 20);
static_assert(offsetof(RndConfig, mAsyncComputeEnabled) == 177);
static_assert(offsetof(RndConfig, mLightTileSize) == 32);
static_assert(offsetof(RndConfig, mLightTileDepthSlices) == 40);
static_assert(offsetof(RndConfig, mVolumetricScatteringTileSize) == 48);
static_assert(offsetof(RndConfig, mMaxLightsPerTile) == 56);
static_assert(offsetof(RndConfig, mMaxPointLights) == 64);
static_assert(offsetof(RndConfig, mMaxSpotLights) == 72);
static_assert(offsetof(RndConfig, mMaxDirectionalLights) == 80);
static_assert(offsetof(RndConfig, mMaxLightProbes) == 88);
static_assert(offsetof(RndConfig, mUseLightProbes) == 96);
static_assert(offsetof(RndConfig, mMaxPartialFramerateScenes) == 104);
static_assert(offsetof(RndConfig, mMaxShadowContribBuffers) == 112);
static_assert(offsetof(RndConfig, mShadowSoftenTileSize) == 120);
static_assert(offsetof(RndConfig, mMaskTileSize) == 128);
static_assert(offsetof(RndConfig, mOutputResolution) == 136);
static_assert(offsetof(RndConfig, mQualityLevel) == 148);
static_assert(offsetof(RndConfig, mVSyncEnabled) == 152);
static_assert(offsetof(RndConfig, mFirstDrawnScene) == 160);
static_assert(offsetof(RndConfig, mMultithreadedRenderingEnabled) == 176);
static_assert(offsetof(RndConfig, mScreenshotResolution) == 184);
static_assert(offsetof(RndConfig, mMaxGeoOverdraw) == 192);
static_assert(offsetof(RndConfig, mGraphicsApiValidationEnabled) == 216);
static_assert(offsetof(RndConfig, mGenerateShaderDebugInfo) == 224);
static_assert(sizeof(RndConfig) == 232);

// Parses "<width>x<height>", or "<height>" for a 16:9 resolution. Name not
// in the reference map.
bool ParseResolution(const char* text, Vector2i& extent);  // 0x441940

// The graphics API the platform's config block selects, or kGfxApiNull.
// Name not in the reference map.
HxGfxApi RndGfxApiForPlatform(HxPlatform platform);  // 0x4414A0

// Names not in the reference map.
const char* RndQualityLevelName(RndQualityLevel level);  // 0x442520
RndQualityLevel RndQualityLevelFromName(const char* name);  // 0x442540
