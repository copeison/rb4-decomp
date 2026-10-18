#include "render/postprocessing/antialiasing/RndCShaderCMAAFinalProcess.h"

#include "render/context/RndContext.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x451010; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gFinalProcessGroupSizeUnused = -1;  // 0x1A755F0
int gFinalProcessGroupSize2D = 8;                        // 0x1A755F4
[[maybe_unused]] int gFinalProcessGroupSize3D = 4;       // 0x1A755F8

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

// Reconstructed from eboot.elf at 0x450BF0.
RndCShaderCMAAFinalProcess::RndCShaderCMAAFinalProcess()
    : mInputColor(-1),
      mInputEdges(-1),
      mOutputBuffer(-1),
      mTargetSizeInv(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x450C30.
RndCShaderCMAAFinalProcess::~RndCShaderCMAAFinalProcess() {}

// Reconstructed from eboot.elf at 0x450C60. The groups cover the compressed
// edges, like the earlier stages.
void RndCShaderCMAAFinalProcess::Dispatch(
    RndContext& context,
    RndBufferCollection& buffers,
    const RndSceneDrawTarget& target,
    unsigned long parity) {
    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);

    RndTextureBase* color = buffers.mCMAAColor;
    if (color == nullptr) {
        color = LightAccum(buffers, target.mDstLightAccum);
    }
    color->Select(context, kShaderProgramCompute, mInputColor, 0, 0);
    buffers.mCMAAEdges[parity == 0]->Select(
        context, kShaderProgramCompute, mInputEdges, 0, 0);
    RndTextureBase* const output = LightAccum(buffers, target.mSrcLightAccum);
    output->Select(
        context, kShaderProgramCompute, mOutputBuffer, kReadWrite, 0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* sizeInv = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTargetSizeInv));
    sizeInv[0] = 1.0F / static_cast<float>(output->mBaseDesc.mWidth);
    sizeInv[1] = 1.0F / static_cast<float>(output->mBaseDesc.mHeight);
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const auto& size = buffers.mCMAACompressedEdges->mBaseDesc;
    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mWidth), gFinalProcessGroupSize2D)),
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mHeight), gFinalProcessGroupSize2D)),
        1);
}

// Reconstructed from eboot.elf at 0x450F10.
const char* RndCShaderCMAAFinalProcess::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CMAAFinalProcess.hlsl";
}

// Reconstructed from eboot.elf at 0x450F20.
void RndCShaderCMAAFinalProcess::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mInputColor = resources.AddTexture(
        "gInputColor",
        "gInputColorSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mInputEdges = resources.AddTexture(
        "gInputEdges",
        "gInputEdgesSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputBuffer = resources.AddTextureWritable(
        "gOutputBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTargetSizeInv =
        cbuffer.AddConstant(kShaderNumericFloat2, "gTargetSizeInv");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x450FF0.
const char* RndCShaderCMAAFinalProcess::_GetClassNameImpl() const {
    return "RndCShaderCMAAFinalProcess";
}

// Reconstructed from eboot.elf at 0x451000. The compute base's body,
// emitted again for this class.
int RndCShaderCMAAFinalProcess::_GetShaderStages() const {
    return kShaderStagesCompute;
}
