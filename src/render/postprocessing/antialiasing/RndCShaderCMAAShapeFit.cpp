#include "render/postprocessing/antialiasing/RndCShaderCMAAShapeFit.h"

#include "render/context/RndContext.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x451430; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gShapeFitGroupSizeUnused = -1;  // 0x1A755FC
int gShapeFitGroupSize2D = 8;                        // 0x1A75600
[[maybe_unused]] int gShapeFitGroupSize3D = 4;       // 0x1A75604

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

// Reconstructed from eboot.elf at 0x451030.
RndCShaderCMAAShapeFit::RndCShaderCMAAShapeFit()
    : mInputEdges(-1),
      mOutputColor(-1),
      mOutputEdges(-1),
      mTargetSizeInv(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x451070.
RndCShaderCMAAShapeFit::~RndCShaderCMAAShapeFit() {}

// Reconstructed from eboot.elf at 0x4510A0. The groups cover the compressed
// edges, like the earlier stages.
void RndCShaderCMAAShapeFit::Dispatch(
    RndContext& context,
    RndBufferCollection& buffers,
    const RndSceneDrawTarget& target,
    unsigned long parity) {
    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);

    buffers.mCMAAEdges[parity != 0]->Select(
        context, kShaderProgramCompute, mInputEdges, 0, 0);
    RndTextureBase* const color = LightAccum(buffers, target.mSrcLightAccum);
    color->Select(context, kShaderProgramCompute, mOutputColor, kReadWrite, 0);
    buffers.mCMAAEdges[parity == 0]->Select(
        context, kShaderProgramCompute, mOutputEdges, kReadWrite, 0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* sizeInv = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTargetSizeInv));
    sizeInv[0] = 1.0F / static_cast<float>(color->mBaseDesc.mWidth);
    sizeInv[1] = 1.0F / static_cast<float>(color->mBaseDesc.mHeight);
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const auto& size = buffers.mCMAACompressedEdges->mBaseDesc;
    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mWidth), gShapeFitGroupSize2D)),
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mHeight), gShapeFitGroupSize2D)),
        1);
}

// Reconstructed from eboot.elf at 0x451340.
const char* RndCShaderCMAAShapeFit::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CMAAShapeFit.hlsl";
}

// Reconstructed from eboot.elf at 0x451350.
void RndCShaderCMAAShapeFit::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mInputEdges = resources.AddTexture(
        "gInputEdges",
        "gInputEdgesSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputColor = resources.AddTextureWritable(
        "gOutputColor",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputEdges = resources.AddTextureWritable(
        "gOutputEdges",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTargetSizeInv =
        cbuffer.AddConstant(kShaderNumericFloat2, "gTargetSizeInv");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x451410.
const char* RndCShaderCMAAShapeFit::_GetClassNameImpl() const {
    return "RndCShaderCMAAShapeFit";
}

// Reconstructed from eboot.elf at 0x451420. The compute base's body,
// emitted again for this class.
int RndCShaderCMAAShapeFit::_GetShaderStages() const {
    return kShaderStagesCompute;
}
