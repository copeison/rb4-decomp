#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgeDetect.h"

#include "render/context/RndContext.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x450890; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gEdgeDetectGroupSizeUnused = -1;  // 0x1A755D8
int gEdgeDetectGroupSize2D = 8;                        // 0x1A755DC
[[maybe_unused]] int gEdgeDetectGroupSize3D = 4;       // 0x1A755E0

// Names not in the reference map.
constexpr unsigned int kReadWrite = RndShaderResource::kSelectReadWrite;

// The light accumulation buffer the target selects, or the frame
// interval's own one outside a scene context.
RndTextureBase* LightAccum(
    const RndBufferCollection& buffers,
    unsigned long index) {
    if (buffers.mActiveSceneContext != 0) {
        return buffers.mLightAccum[index];
    }
    return buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval]
        .mPartialLightAccum;
}

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

}  // namespace

// Reconstructed from eboot.elf at 0x450490.
RndCShaderCMAAEdgeDetect::RndCShaderCMAAEdgeDetect()
    : mSourceBuffer(-1),
      mOutputEdgesBuffer(-1),
      mOutputColorBuffer(-1),
      mEdgeThreshold(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x4504D0.
RndCShaderCMAAEdgeDetect::~RndCShaderCMAAEdgeDetect() {}

// Reconstructed from eboot.elf at 0x450500. One thread group covers 8x8
// compressed edge texels.
void RndCShaderCMAAEdgeDetect::Dispatch(
    RndContext& context,
    RndBufferCollection& buffers,
    const RndSceneDrawTarget& target,
    float edgeThreshold) {
    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);

    LightAccum(buffers, target.mSrcLightAccum)
        ->Select(context, kShaderProgramCompute, mSourceBuffer, 0, 0);
    buffers.mCMAACompressedEdges->Select(
        context, kShaderProgramCompute, mOutputEdgesBuffer, kReadWrite, 0);
    RndTextureBase* color = buffers.mCMAAColor;
    if (color == nullptr) {
        color = LightAccum(buffers, target.mDstLightAccum);
    }
    color->Select(
        context, kShaderProgramCompute, mOutputColorBuffer, kReadWrite, 0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    *static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mEdgeThreshold)) =
        edgeThreshold;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const auto& size = buffers.mCMAACompressedEdges->mBaseDesc;
    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mWidth), gEdgeDetectGroupSize2D)),
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mHeight), gEdgeDetectGroupSize2D)),
        1);
}

// Reconstructed from eboot.elf at 0x450770.
const char* RndCShaderCMAAEdgeDetect::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CMAAEdgeDetect.hlsl";
}

// Reconstructed from eboot.elf at 0x450780.
void RndCShaderCMAAEdgeDetect::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSourceBuffer = resources.AddTexture(
        "gSourceBuffer",
        "gSourceBufferSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputEdgesBuffer = resources.AddTextureWritable(
        "gOutputEdgesBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt4);
    mOutputColorBuffer = resources.AddTextureWritable(
        "gOutputColorBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mEdgeThreshold = cbuffer.AddConstant(kShaderNumericFloat, "gEdgeThreshold");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x450840.
const char* RndCShaderCMAAEdgeDetect::_GetClassNameImpl() const {
    return "RndCShaderCMAAEdgeDetect";
}

// Reconstructed from eboot.elf at 0x450860. The compute base's body,
// emitted again for this class.
int RndCShaderCMAAEdgeDetect::_GetShaderStages() const {
    return kShaderStagesCompute;
}
