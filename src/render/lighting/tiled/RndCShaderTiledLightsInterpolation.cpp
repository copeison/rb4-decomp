#include "render/lighting/tiled/RndCShaderTiledLightsInterpolation.h"

#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndContext.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Thread-group widths, set by a static initializer at 0x6DA530; only the 2D
// width is used. Names not in the reference map.
int gInterpGroupSizeUnused = -1;  // 0x1AB18F0
int gInterpGroupSize2D = 8;  // 0x1AB18F4
int gInterpGroupSize3D = 4;  // 0x1AB18F8
constexpr unsigned long kComputeKey = 4;

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient + (quotient * divisor < count ? 1 : 0);
}

}  // namespace

// Reconstructed from eboot.elf at 0x6D9F10.
RndCShaderTiledLightsInterpolation::RndCShaderTiledLightsInterpolation()
    : mShadingMode{},
      mSrcLightAccumBuffer(-1),
      mGBuffer0(-1),
      mLightInterpBuffer(-1),
      mDstLightAccumBuffer(-1),
      mCBufferSize(0),
      mDimensions(-1) {}

RndCShaderTiledLightsInterpolation::~RndCShaderTiledLightsInterpolation() {}

// Reconstructed from eboot.elf at 0x6D9FA0. The interpolation input is the
// first frame interval's, whichever interval is active.
void RndCShaderTiledLightsInterpolation::Dispatch(
    RndContext& context,
    RndBufferCollection& buffers,
    const RndSceneDrawParams& drawParams,
    RndTextureBase& srcLightAccum) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Tiled Lights Interp");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndShaderKeyGroup keys{};
    keys.mKeys[kComputeKey] = mShadingMode.SetValue(
        0, static_cast<unsigned int>(context.mShadingMode));
    _SelectShaderCollection(context, keys);

    srcLightAccum.Select(
        context,
        kShaderProgramCompute,
        mSrcLightAccumBuffer,
        RndShaderResource::kSelectNoSampler,
        0);
    buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval].mGBufferColor->Select(
        context,
        kShaderProgramCompute,
        mGBuffer0,
        RndShaderResource::kSelectNoSampler,
        0);
    buffers.mFrameIntervals.mData[0].mTiledLightInterp->Select(
        context,
        kShaderProgramCompute,
        mLightInterpBuffer,
        RndShaderResource::kSelectNoSampler,
        0);
    auto* dstLightAccum = buffers.mActiveSceneContext != 0
        ? buffers.mLightAccum[drawParams.mSceneContext]
        : buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval]
              .mPartialLightAccum;
    dstLightAccum->Select(
        context,
        kShaderProgramCompute,
        mDstLightAccumBuffer,
        RndShaderResource::kSelectReadWrite,
        0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto& size = buffers.mSize;
    auto* dimensions = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mDimensions));
    dimensions[0] = static_cast<float>(size.x);
    dimensions[1] = static_cast<float>(size.y);
    dimensions[2] = 1.0F / static_cast<float>(size.x);
    dimensions[3] = 1.0F / static_cast<float>(size.y);
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    (void)gInterpGroupSizeUnused;
    (void)gInterpGroupSize3D;
    context._DispatchComputeImpl(
        static_cast<unsigned int>(
            CeilDiv(CeilDiv(size.x, 2), gInterpGroupSize2D)),
        static_cast<unsigned int>(
            CeilDiv(CeilDiv(size.y, 2), gInterpGroupSize2D)),
        1);
}

const char* RndCShaderTiledLightsInterpolation::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/TiledLightsInterpolation.hlsl";
}

// Reconstructed from eboot.elf at 0x6DA350.
void RndCShaderTiledLightsInterpolation::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mShadingMode = defines.GetDefines(kShaderProgramCompute)
                       .Add(Symbol("HX_SHADING_MODE"), 0, 19);
    mSrcLightAccumBuffer = resources.AddTexture(
        "gSrcLightAccumBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mGBuffer0 = resources.AddTexture(
        "gGBuffer0",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mLightInterpBuffer = resources.AddTexture(
        "gLightInterpBuffer",
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
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x6DA4C0. Compute programs exist only for
// the standard and debug shading modes.
bool RndCShaderTiledLightsInterpolation::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (type == kShaderProgramCompute) {
        const auto shadingMode = mShadingMode.GetValue(key);
        if (shadingMode != static_cast<unsigned int>(kShadingModeDebugMisc) &&
            shadingMode != static_cast<unsigned int>(kShadingModeStandard)) {
            return false;
        }
    }
    return true;
}

const char* RndCShaderTiledLightsInterpolation::_GetClassNameImpl() const {
    return "RndCShaderTiledLightsInterpolation";
}
