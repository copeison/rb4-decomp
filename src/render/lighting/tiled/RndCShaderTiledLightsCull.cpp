#include "render/lighting/tiled/RndCShaderTiledLightsCull.h"

#include <cmath>
#include <cstring>

#include "render/buffers/RndComputeBuffer.h"
#include "render/buffers/RndShaderCBuffer.h"
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
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Threads in one tiled-lighting group. Name not in the reference map.
constexpr unsigned long kThreadsPerGroup = 64;

// Thread-group widths, set by a static initializer at 0x6D9EA0; only the 2D
// width is used. Names not in the reference map.
[[maybe_unused]] int gCullUnknown = -1;      // 0x1AB18B8
int gCullGroupSize2D = 8;                   // 0x1AB18BC
[[maybe_unused]] int gCullGroupSize3D = 4;  // 0x1AB18C0

// Names not in the reference map.
constexpr unsigned long kComputeKey = 4;
constexpr unsigned int kNoSampler = RndShaderResource::kSelectNoSampler;
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

// Reconstructed from eboot.elf at 0x6D8AF0. Stereo cameras also read the
// right eye's depth ranges and write the stereo lists; culling only depth
// slice zero dispatches fixed 8x8 tile groups. The binary takes the frustum
// widths' square roots with a refined reciprocal square root.
void RndCShaderTiledLightsCull::Dispatch(RndContext& context, Params& params) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Tiled Lights Cull");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    auto* device = TheRndDevice();
    const auto& camera = *params.mCamera;
    auto& buffers = *params.mBuffers;
    const bool stereo = camera.mTargetMode == kTargetModeStereo;
    const float width = camera.mViewportSize.x;
    const float height = camera.mViewportSize.y;
    const int tileSize = static_cast<int>(device->mSettings->mLightTileSize);
    const int tilesX = CeilDiv(static_cast<int>(width), tileSize);
    const int tilesY = CeilDiv(static_cast<int>(height), tileSize);

    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] = mOnlySliceZero.SetValue(
        mIsStereo.SetValue(0, stereo ? 1U : 0U), params.mOnlySliceZero ? 1U : 0U);
    _SelectShaderCollection(context, keys);

    auto* lights = params.mLightBuffers;
    SelectCompute(context, *lights[kTiledLightsPoint].mBuffer, mPointLights);
    SelectCompute(context, *lights[kTiledLightsSpot].mBuffer, mSpotLights);
    SelectCompute(context, *params.mLightProbes, mLightProbes);
    if (params.mOnlySliceZero) {
        SelectCompute(context, *params.mSliceZeroLightIds, mSliceZeroLightIds);
    }
    auto& frame = buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
    SelectCompute(context, *frame.mTiledDepthRange, mTiledDepthRangeBuffer, kNoSampler);
    if (stereo) {
        auto& rightEye = *params.mRightEyeBuffers;
        SelectCompute(
            context,
            *rightEye.mFrameIntervals.mData[rightEye.mActiveFrameInterval].mTiledDepthRange,
            mRightEyeTiledDepthRangeBuffer);
    }
    // Without the scene mask the binary binds the device field at +0x808,
    // the black 3D default texture.
    RndTextureBase* sceneMask = params.mUseSceneMask
        ? buffers.mTiledSceneMask[1]
        : device->mDefaults.mTextures[kDefaultTextureBlack].mTexture3D;
    SelectCompute(context, *sceneMask, mSceneMask, kNoSampler);
    SelectCompute(context, *device->mLighting.mTiledLightIdsCount, mLightIdsCount, kReadWrite);
    if (stereo) {
        SelectCompute(context, *frame.mStereoTiledLightIds[0], mLightIds, kReadWrite);
        SelectCompute(context, *frame.mStereoTiledLightIds[1], mScratchLightIds, kReadWrite);
        SelectCompute(context, *frame.mStereoTiledLightIdRanges, mLightIdRanges, kReadWrite);
    } else {
        SelectCompute(context, *frame.mTiledLightIds[0], mLightIds, kReadWrite);
        SelectCompute(context, *frame.mTiledLightIds[1], mScratchLightIds, kReadWrite);
        SelectCompute(context, *frame.mTiledLightIdRanges, mLightIdRanges, kReadWrite);
    }

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const float numPosLights[2] = {
        static_cast<float>(lights[kTiledLightsPoint].mNumPosLights),
        static_cast<float>(lights[kTiledLightsSpot].mNumPosLights),
    };
    const float numNegLights[2] = {
        static_cast<float>(lights[kTiledLightsPoint].mNumNegLights),
        static_cast<float>(lights[kTiledLightsSpot].mNumNegLights),
    };
    // The probes are bound a second time before they are counted.
    auto& probes = *params.mLightProbes;
    SelectCompute(context, probes, mLightProbes);
    const unsigned long numProbes = probes.mStagingSize / probes.mDesc.mElementSize;

    auto* dimensions = CBufferFloats(cbuffer, mDimensions);
    dimensions[0] = width;
    dimensions[1] = height;
    dimensions[2] = 1.0F / width;
    dimensions[3] = 1.0F / height;
    cbuffer.mSyncPending = true;
    // The binary rereads the camera width for stereo; one eye has the same
    // dimensions here.
    const float monoWidth = stereo ? camera.mViewportSize.x : width;
    auto* monoDimensions = CBufferFloats(cbuffer, mMonoDimensions);
    monoDimensions[0] = monoWidth;
    monoDimensions[1] = height;
    monoDimensions[2] = 1.0F / monoWidth;
    monoDimensions[3] = 1.0F / height;
    cbuffer.mSyncPending = true;
    auto* tileCounts = CBufferFloats(cbuffer, mTileCounts);
    tileCounts[0] = static_cast<float>(tilesX);
    tileCounts[1] = static_cast<float>(tilesY);
    tileCounts[2] = 0.0F;
    tileCounts[3] = 0.0F;
    if (stereo) {
        tileCounts[2] = static_cast<float>(
            CeilDiv(static_cast<int>(camera.mViewportSize.x), tileSize));
        tileCounts[3] = static_cast<float>(
            CeilDiv(static_cast<int>(camera.mViewportSize.y), tileSize));
    }
    auto* posLights = CBufferFloats(cbuffer, mNumPosLights);
    posLights[0] = numPosLights[0];
    posLights[1] = numPosLights[1];
    auto* negLights = CBufferFloats(cbuffer, mNumNegLights);
    negLights[0] = numNegLights[0];
    negLights[1] = numNegLights[1];
    *CBufferFloats(cbuffer, mNumProbes) = static_cast<float>(numProbes);
    cbuffer.mSyncPending = true;

    if (params.mOnlySliceZero) {
        auto* sliceZeroPos = CBufferFloats(cbuffer, mNumSliceZeroPosLights);
        sliceZeroPos[0] = static_cast<float>(params.mNumSliceZeroLights[0][0]);
        sliceZeroPos[1] = static_cast<float>(params.mNumSliceZeroLights[1][0]);
        auto* sliceZeroNeg = CBufferFloats(cbuffer, mNumSliceZeroNegLights);
        sliceZeroNeg[0] = static_cast<float>(params.mNumSliceZeroLights[0][1]);
        sliceZeroNeg[1] = static_cast<float>(params.mNumSliceZeroLights[1][1]);
        *CBufferFloats(cbuffer, mNumSliceZeroProbes) =
            static_cast<float>(params.mNumSliceZeroProbes);
        cbuffer.mSyncPending = true;
    }

    const auto& forward = camera.mPrimaryView.mWorldXfm.m.y;
    auto* camDir = CBufferFloats(cbuffer, mCamDir);
    camDir[0] = forward.x;
    camDir[1] = forward.y;
    camDir[2] = forward.z;
    const auto& settings = camera.GetCameraSettings();
    const float nearPlane = settings.mNearPlane;
    const float farPlane = settings.mFarPlane;
    auto* nearFar = CBufferFloats(cbuffer, mCamNearFarParams);
    nearFar[0] = nearPlane;
    nearFar[1] = farPlane;
    nearFar[2] = farPlane * nearPlane;
    nearFar[3] = farPlane - nearPlane;
    const float depthScale = 1.0F / (farPlane - nearPlane);
    auto* depthRange = CBufferFloats(cbuffer, mCamDepthRangeParams);
    depthRange[0] = depthScale * farPlane;
    depthRange[1] = -(nearPlane * depthScale);
    cbuffer.mSyncPending = true;

    float frustumCorners[8][4] = {};
    const auto* corners = camera.mFrustum.mCorners.mData;
    for (int i = 0; i < 8; ++i) {
        frustumCorners[i][0] = corners[i].x;
        frustumCorners[i][1] = corners[i].y;
        frustumCorners[i][2] = corners[i].z;
    }
    std::memcpy(
        CBufferFloats(cbuffer, mFrustumCorners), frustumCorners, sizeof(frustumCorners));
    cbuffer.mSyncPending = true;

    if (stereo) {
        const auto* eyeCorners = camera.mViews.mData[0].mWorldFrustum.mCorners.mData;
        auto* widths = CBufferFloats(cbuffer, mFrustumWidths);
        widths[0] = Distance(corners[0], corners[1]);
        widths[1] = Distance(corners[4], corners[5]);
        widths[2] = Distance(eyeCorners[0], eyeCorners[1]);
        widths[3] = Distance(eyeCorners[4], eyeCorners[5]);
        cbuffer.mSyncPending = true;
    }
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    if (params.mOnlySliceZero) {
        context._DispatchComputeImpl(
            static_cast<unsigned int>(CeilDiv(tilesX, gCullGroupSize2D)),
            static_cast<unsigned int>(CeilDiv(tilesY, gCullGroupSize2D)),
            1);
    } else {
        context._DispatchComputeImpl(
            static_cast<unsigned int>(CeilDiv(tilesX, mTilesPerThreadGroup.x)),
            static_cast<unsigned int>(CeilDiv(tilesY, mTilesPerThreadGroup.y)),
            1);
    }
}

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
