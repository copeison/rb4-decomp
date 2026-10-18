#include "render/postprocessing/antialiasing/RndCShaderCMAAEdgePrune.h"

#include "render/context/RndContext.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x450BD0; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gEdgePruneGroupSizeUnused = -1;  // 0x1A755E4
int gEdgePruneGroupSize2D = 8;                        // 0x1A755E8
[[maybe_unused]] int gEdgePruneGroupSize3D = 4;       // 0x1A755EC

// Names not in the reference map.
constexpr unsigned int kReadWrite = RndShaderResource::kSelectReadWrite;

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

}  // namespace

// Reconstructed from eboot.elf at 0x4508B0.
RndCShaderCMAAEdgePrune::RndCShaderCMAAEdgePrune()
    : mSourceEdges(-1),
      mOutputEdges(-1),
      mNonDominantEdgeThreshold(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x450900.
RndCShaderCMAAEdgePrune::~RndCShaderCMAAEdgePrune() {}

// Reconstructed from eboot.elf at 0x450930.
void RndCShaderCMAAEdgePrune::Dispatch(
    RndContext& context,
    RndBufferCollection& buffers,
    unsigned long parity,
    float nonDominantEdgeThreshold) {
    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);

    buffers.mCMAACompressedEdges->Select(
        context, kShaderProgramCompute, mSourceEdges, 0, 0);
    buffers.mCMAAEdges[parity != 0]->Select(
        context, kShaderProgramCompute, mOutputEdges, kReadWrite, 0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    *static_cast<float*>(RndShaderDrawUtl::GetCBufferMember(
        cbuffer, mNonDominantEdgeThreshold)) = nonDominantEdgeThreshold;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const auto& size = buffers.mCMAACompressedEdges->mBaseDesc;
    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mWidth), gEdgePruneGroupSize2D)),
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(size.mHeight), gEdgePruneGroupSize2D)),
        1);
}

// Reconstructed from eboot.elf at 0x450B00.
const char* RndCShaderCMAAEdgePrune::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CMAAEdgePrune.hlsl";
}

// Reconstructed from eboot.elf at 0x450B10.
void RndCShaderCMAAEdgePrune::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSourceEdges = resources.AddTexture(
        "gSourceEdges",
        "gSourceEdgesSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt4);
    mOutputEdges = resources.AddTextureWritable(
        "gOutputEdges",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mNonDominantEdgeThreshold = cbuffer.AddConstant(
        kShaderNumericFloat, "gNonDominantEdgeThreshold");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x450BB0.
const char* RndCShaderCMAAEdgePrune::_GetClassNameImpl() const {
    return "RndCShaderCMAAEdgePrune";
}

// Reconstructed from eboot.elf at 0x450BC0. The compute base's body,
// emitted again for this class.
int RndCShaderCMAAEdgePrune::_GetShaderStages() const {
    return kShaderStagesCompute;
}
