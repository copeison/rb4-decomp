#include "render/lighting/volumetric/RndCShaderVScatAccumScattering.h"

#include <cmath>

#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTexture3D.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x6D26B0; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gAccumScatteringGroupSizeUnused = -1;  // 0x1AB15C0
int gAccumScatteringGroupSize2D = 8;                        // 0x1AB15C4
[[maybe_unused]] int gAccumScatteringGroupSize3D = 4;       // 0x1AB15C8

// Names not in the reference map.
constexpr unsigned long kComputeKey = 4;

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

// A 3D texture's extent, in texels.
void SetDimensions(float* constant, const RndTextureBase& texture) {
    const auto& desc = texture.mBaseDesc;
    constant[0] = static_cast<float>(static_cast<int>(desc.mWidth));
    constant[1] = static_cast<float>(static_cast<int>(desc.mHeight));
    constant[2] = static_cast<float>(static_cast<int>(desc.mDepth));
}

float Distance(const Vector3& a, const Vector3& b) {
    const float x = a.x - b.x;
    const float y = a.y - b.y;
    const float z = a.z - b.z;
    return std::sqrt(x * x + y * y + z * z);
}

}  // namespace

// Reconstructed from eboot.elf at 0x6D1E70.
RndCShaderVScatAccumScattering::RndCShaderVScatAccumScattering()
    : mUseSceneMask{},
      mIsStereo{},
      mStereoEye{},
      mDensityInscatteringTex(-1),
      mTiledDepthRangeBuffer(-1),
      mSceneMask(-1),
      mOutputTex(-1),
      mSrcDimensions(-1),
      mDstDimensions(-1),
      mScreenDimensions(-1),
      mDepthFracOffsetParams(-1),
      mFrustumParams(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x6D1EF0.
RndCShaderVScatAccumScattering::~RndCShaderVScatAccumScattering() {}

// Reconstructed from eboot.elf at 0x6D1F20. One thread per output column;
// the shader marches it through the volume's depth.
void RndCShaderVScatAccumScattering::Dispatch(
    RndContext& context,
    Params& params) {
    RndShaderKey key =
        mUseSceneMask.SetValue(0, params.mSceneMask != nullptr);
    if (params.mStereoEye == -1) {
        key = mIsStereo.SetValue(key, 0);
    } else {
        key = mIsStereo.SetValue(key, 1);
        key = mStereoEye.SetValue(
            key, static_cast<unsigned int>(params.mStereoEye));
    }
    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] = key;
    _SelectShaderCollection(context, keys);

    params.mDensityInscattering->Select(
        context, kShaderProgramCompute, mDensityInscatteringTex, 0, 0);
    params.mTiledDepthRange->Select(
        context, kShaderProgramCompute, mTiledDepthRangeBuffer, 0, 0);
    if (params.mSceneMask != nullptr) {
        params.mSceneMask->Select(
            context, kShaderProgramCompute, mSceneMask, 0, 0);
    }
    params.mOutput->Select(
        context,
        kShaderProgramCompute,
        mOutputTex,
        RndShaderResource::kSelectReadWrite,
        0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto member = [&cbuffer](unsigned long offset) {
        return static_cast<float*>(
            RndShaderDrawUtl::GetCBufferMember(cbuffer, offset));
    };
    SetDimensions(member(mSrcDimensions), *params.mDensityInscattering);
    SetDimensions(member(mDstDimensions), *params.mOutput);
    float* const screen = member(mScreenDimensions);
    screen[0] = static_cast<float>(params.mScreenSize.x);
    screen[1] = static_cast<float>(params.mScreenSize.y);

    // Maps between the camera's linear depth and the fraction of the fog
    // range the volume covers.
    const RndCameraSettings& camera = params.mCamera->GetCameraSettings();
    const float range = params.mEndDist - params.mStartDist;
    const float invRange = 1.0F / range;
    const float invClipRange = 1.0F / (camera.mFarPlane - camera.mNearPlane);
    float* const depthFrac = member(mDepthFracOffsetParams);
    depthFrac[0] = invRange * camera.mFarPlane;
    depthFrac[1] = -(params.mStartDist * invRange);
    depthFrac[2] = invClipRange * range;
    depthFrac[3] = invClipRange * (params.mStartDist - camera.mNearPlane);

    // A stereo eye also takes the widths of the top edges of the near and
    // far planes: the eye's own frustum's, then the first view's world
    // frustum's.
    if (params.mStereoEye != -1) {
        const Vector3* const eye =
            params.mStereoCamera->mFrustum.mCorners.mData;
        const Vector3* const view =
            params.mCamera->mViews.mData[0].mWorldFrustum.mCorners.mData;
        float* const frustum = member(mFrustumParams);
        frustum[0] = Distance(eye[0], eye[1]);
        frustum[1] = Distance(eye[4], eye[5]);
        frustum[2] = Distance(view[0], view[1]);
        frustum[3] = Distance(view[4], view[5]);
    }
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const auto& size = params.mOutput->mBaseDesc;
    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mWidth), gAccumScatteringGroupSize2D)),
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mHeight), gAccumScatteringGroupSize2D)),
        1);
}

// Reconstructed from eboot.elf at 0x6D23F0.
const char* RndCShaderVScatAccumScattering::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/VScatAccumScattering.hlsl";
}

// Reconstructed from eboot.elf at 0x6D2400.
void RndCShaderVScatAccumScattering::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto* device = TheRndDevice();
    const auto* settings = device != nullptr ? device->mSettings : nullptr;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"),
        static_cast<int>(
            settings != nullptr ? settings->mLightTileSize
                                : RndConfig::kDefaultLightTileSize));

    auto& compute = defines.GetDefines(kShaderProgramCompute);
    mUseSceneMask = compute.AddBool(Symbol("HX_USE_SCENE_MASK"));
    mIsStereo = compute.AddBool(Symbol("HX_IS_STEREO"));
    mStereoEye = compute.Add(Symbol("HX_STEREO_EYE"), 0, 2);

    mDensityInscatteringTex = resources.AddTexture(
        "gDensityInscatteringTex",
        "gDensityInscatteringTexSampler",
        RndTextureBase::kTexture3D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTiledDepthRangeBuffer = resources.AddTexture(
        "gTiledDepthRangeBuffer",
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
    mOutputTex = resources.AddTextureWritable(
        "gOutputTex",
        RndTextureBase::kTexture3D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSrcDimensions =
        cbuffer.AddConstant(kShaderNumericFloat3, "gSrcDimensions");
    mDstDimensions =
        cbuffer.AddConstant(kShaderNumericFloat3, "gDstDimensions");
    mScreenDimensions =
        cbuffer.AddConstant(kShaderNumericFloat2, "gScreenDimensions");
    mDepthFracOffsetParams =
        cbuffer.AddConstant(kShaderNumericFloat4, "gDepthFracOffsetParams");
    mFrustumParams =
        cbuffer.AddConstant(kShaderNumericFloat4, "gFrustumParams");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x6D26A0.
const char* RndCShaderVScatAccumScattering::_GetClassNameImpl() const {
    return "RndCShaderVScatAccumScattering";
}
