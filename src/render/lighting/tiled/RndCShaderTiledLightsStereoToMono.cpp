#include "render/lighting/tiled/RndCShaderTiledLightsStereoToMono.h"

#include <cmath>
#include <cstring>

#include "render/buffers/RndComputeBuffer.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/lighting/tiled/RndCShaderTiledLightsCull.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Name not in the reference map.
constexpr unsigned int kReadWrite = RndShaderResource::kSelectReadWrite;

void SelectCompute(
    RndContext& context,
    RndShaderResource& resource,
    unsigned long slot,
    unsigned int flags = 0) {
    resource.Select(context, kShaderProgramCompute, slot, flags, 0);
}

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient + (quotient * divisor < count ? 1 : 0);
}

float* CBufferFloats(RndShaderCBuffer& cbuffer, unsigned long memberOffset) {
    return static_cast<float*>(RndShaderDrawUtl::GetCBufferMember(cbuffer, memberOffset));
}

float Distance(const Vector3& a, const Vector3& b) {
    const float x = a.x - b.x;
    const float y = a.y - b.y;
    const float z = a.z - b.z;
    return std::sqrt(x * x + y * y + z * z);
}

}  // namespace

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

// Reconstructed from eboot.elf at 0x6DA5D0. The camera constants come from
// this eye's camera while the shader runs. The eye index is the target
// mode's offset from the left-eye mode. The binary takes the frustum widths'
// square roots with a refined reciprocal square root.
void RndCShaderTiledLightsStereoToMono::Dispatch(RndContext& context, const Params& params) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Tiled Lights Stereo2Mono");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
    const auto* previousCamera = context.mCameraCBufferOverride;
    context.SetCameraCBufferOverrideContext(params.mCamera);

    auto* device = TheRndDevice();
    const auto& camera = *params.mCamera;
    auto& bothEyes = *params.mBothEyesBuffers;
    auto& buffers = *params.mBuffers;
    const auto* settings = device->mSettings;
    const float width = camera.mViewportSize.x;
    const float height = camera.mViewportSize.y;
    const int tileSize = static_cast<int>(settings->mLightTileSize);
    const int tilesX = CeilDiv(static_cast<int>(width), tileSize);
    const int tilesY = CeilDiv(static_cast<int>(height), tileSize);

    auto& bothEyesFrame = bothEyes.mFrameIntervals.mData[bothEyes.mActiveFrameInterval];
    SelectCompute(context, *bothEyesFrame.mStereoTiledLightIds[0], mBothEyesLightIds);
    SelectCompute(context, *bothEyesFrame.mStereoTiledLightIdRanges, mBothEyesLightIdRanges);
    // Without the scene mask the binary binds the device field at +0x808,
    // the zero 2D default texture.
    RndTextureBase* sceneMask = params.mUseSceneMask
        ? buffers.mTiledSceneMask[1]
        : device->mDefaults.mTextures2D[kDefaultTextureZero];
    SelectCompute(context, *sceneMask, mSceneMask);
    SelectCompute(context, *device->mLighting.mTiledLightIdsCount, mLightIdsCount, kReadWrite);
    auto& frame = buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
    SelectCompute(context, *frame.mTiledLightIds[0], mCurEyeLightIds, kReadWrite);
    SelectCompute(context, *frame.mTiledLightIds[1], mCurEyeScratchLightIds, kReadWrite);
    SelectCompute(context, *frame.mTiledLightIdRanges, mCurEyeLightIdRanges, kReadWrite);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    *CBufferFloats(cbuffer, mWhichEye) =
        static_cast<float>(buffers.mTargetMode - kTargetModeLeftEye);
    const float dimensions[4] = {
        1.0F / width,
        1.0F / height,
        static_cast<float>(tilesX),
        static_cast<float>(tilesY),
    };
    std::memcpy(CBufferFloats(cbuffer, mBothEyesDimensions), dimensions, sizeof(dimensions));
    std::memcpy(CBufferFloats(cbuffer, mOneEyeDimensions), dimensions, sizeof(dimensions));
    cbuffer.mSyncPending = true;

    const auto* bothEyesCorners = params.mBothEyesCamera->mFrustum.mCorners.mData;
    const auto* eyeCorners = camera.mViews.mData[0].mWorldFrustum.mCorners.mData;
    auto* widths = CBufferFloats(cbuffer, mFrustumWidths);
    widths[0] = Distance(bothEyesCorners[0], bothEyesCorners[1]);
    widths[1] = Distance(bothEyesCorners[4], bothEyesCorners[5]);
    widths[2] = Distance(eyeCorners[0], eyeCorners[1]);
    widths[3] = Distance(eyeCorners[4], eyeCorners[5]);
    cbuffer.mSyncPending = true;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const auto tilesPerGroup = CalcTilesPerThreadGroup(
        static_cast<unsigned long>(settings->mLightTileDepthSlices));
    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(tilesX, tilesPerGroup.x)),
        static_cast<unsigned int>(CeilDiv(tilesY, tilesPerGroup.y)),
        1);
    context.SetCameraCBufferOverrideContext(previousCamera);
}

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
