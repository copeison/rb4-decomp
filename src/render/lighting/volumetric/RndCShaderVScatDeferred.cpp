#include "render/lighting/volumetric/RndCShaderVScatDeferred.h"

#include "render/context/RndCameraContext.h"
#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x6D3CA0; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gVScatDeferredGroupSizeUnused = -1;  // 0x1AB15F0
int gVScatDeferredGroupSize2D = 8;                        // 0x1AB15F4
[[maybe_unused]] int gVScatDeferredGroupSize3D = 4;       // 0x1AB15F8

// Names not in the reference map.
constexpr unsigned long kComputeKey = 4;
// The fog scale when the fog range is empty.
constexpr float kDegenerateFogScale = 10000.0F;

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6D3490. The constructor leaves the
// scene-mask index unset; _InitConfigImpl assigns it.
RndCShaderVScatDeferred::RndCShaderVScatDeferred()
    : mUseSceneMask{},
      mAccumScatteringTex(-1),
      mSrcLightAccumBuffer(-1),
      mLinearDepthTex(-1),
      mDstLightAccumBuffer(-1),
      mDimensions(-1),
      mProj(-1),
      mDepthFracOffsetParams(-1),
      mFogEndDistance(-1),
      mFogDensity(-1),
      mTexelSize(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x6D3500.
RndCShaderVScatDeferred::~RndCShaderVScatDeferred() {}

// Reconstructed from eboot.elf at 0x6D3530. One thread per light
// accumulation texel.
void RndCShaderVScatDeferred::Dispatch(
    RndContext& context,
    RndTextureBase* accumScattering,
    RndTextureBase* srcLightAccum,
    RndTextureBase* dstLightAccum,
    RndTextureBase* linearDepth,
    float startDist,
    float endDist,
    float fogDensity,
    RndTextureBase* sceneMask) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Apply Volumetric Scattering");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] = mUseSceneMask.SetValue(0, sceneMask != nullptr);
    _SelectShaderCollection(context, keys);

    accumScattering->Select(
        context, kShaderProgramCompute, mAccumScatteringTex, 0, 0);
    srcLightAccum->Select(
        context, kShaderProgramCompute, mSrcLightAccumBuffer, 0, 0);
    linearDepth->Select(context, kShaderProgramCompute, mLinearDepthTex, 0, 0);
    if (sceneMask != nullptr) {
        sceneMask->Select(context, kShaderProgramCompute, mSceneMask, 0, 0);
    }
    dstLightAccum->Select(
        context,
        kShaderProgramCompute,
        mDstLightAccumBuffer,
        RndShaderResource::kSelectReadWrite,
        0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto member = [&cbuffer](unsigned long offset) {
        return static_cast<float*>(
            RndShaderDrawUtl::GetCBufferMember(cbuffer, offset));
    };
    const auto& src = srcLightAccum->mBaseDesc;
    const auto width = static_cast<float>(static_cast<int>(src.mWidth));
    const auto height = static_cast<float>(static_cast<int>(src.mHeight));
    float* const dimensions = member(mDimensions);
    dimensions[0] = width;
    dimensions[1] = height;
    dimensions[2] = 1.0F / width;
    dimensions[3] = 1.0F / height;

    // The main camera's projection, transposed into columns.
    const Hmx::Matrix4& proj = context.mCameras[0].mPrimaryView.mProjection;
    const Vector4* const rows[] = {&proj.x, &proj.y, &proj.z, &proj.w};
    float* const projection = member(mProj);
    for (int row = 0; row < 4; ++row) {
        projection[row] = rows[row]->x;
        projection[4 + row] = rows[row]->y;
        projection[8 + row] = rows[row]->z;
        projection[12 + row] = rows[row]->w;
    }

    // Maps a distance onto [0, 1] across the fog range.
    const float range = endDist - startDist;
    const float scale = range != 0.0F ? 1.0F / range : kDegenerateFogScale;
    float* const depthFrac = member(mDepthFracOffsetParams);
    depthFrac[0] = scale;
    depthFrac[1] = -(scale * startDist);
    *member(mFogEndDistance) = endDist;
    *member(mFogDensity) = fogDensity;

    const auto& volume = accumScattering->mBaseDesc;
    float* const texelSize = member(mTexelSize);
    texelSize[0] = 1.0F / static_cast<float>(static_cast<int>(volume.mWidth));
    texelSize[1] =
        1.0F / static_cast<float>(static_cast<int>(volume.mHeight));
    texelSize[2] = 1.0F / static_cast<float>(static_cast<int>(volume.mDepth));
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(src.mWidth), gVScatDeferredGroupSize2D)),
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(src.mHeight), gVScatDeferredGroupSize2D)),
        1);
}

// Reconstructed from eboot.elf at 0x6D3A30.
const char* RndCShaderVScatDeferred::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/VScatDeferred.hlsl";
}

// Reconstructed from eboot.elf at 0x6D3A40.
void RndCShaderVScatDeferred::_InitConfigImpl(
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
    mUseSceneMask = defines.GetDefines(kShaderProgramCompute)
                        .AddBool(Symbol("HX_USE_SCENE_MASK"));

    mAccumScatteringTex = resources.AddTexture(
        "gAccumScatteringTex",
        "gAccumScatteringTexSampler",
        RndTextureBase::kTexture3D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSrcLightAccumBuffer = resources.AddTexture(
        "gSrcLightAccumBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mLinearDepthTex = resources.AddTexture(
        "gLinearDepthTex",
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
    mDstLightAccumBuffer = resources.AddTextureWritable(
        "gDstLightAccumBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat4, "gDimensions");
    mProj = cbuffer.AddConstant(kShaderNumericFloat4x4, "gProj");
    mDepthFracOffsetParams =
        cbuffer.AddConstant(kShaderNumericFloat2, "gDepthFracOffsetParams");
    mFogEndDistance =
        cbuffer.AddConstant(kShaderNumericFloat, "gFogEndDistance");
    mFogDensity = cbuffer.AddConstant(kShaderNumericFloat, "gFogDensity");
    mTexelSize = cbuffer.AddConstant(kShaderNumericFloat3, "gTexelSize");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x6D3C90.
const char* RndCShaderVScatDeferred::_GetClassNameImpl() const {
    return "RndCShaderVScatDeferred";
}
