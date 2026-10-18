#include "render/postprocessing/blur/RndCShaderBlurClassify.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x62E640; none is used. Names not in the reference map.
[[maybe_unused]] int gBlurClassifyGroupSizeUnused = -1;  // 0x1AAA738
[[maybe_unused]] int gBlurClassifyGroupSize2D = 8;       // 0x1AAA73C
[[maybe_unused]] int gBlurClassifyGroupSize3D = 4;       // 0x1AAA740

// Unbinds the source textures of every stage. Name not in the reference
// map.
void DeselectSourceTextures(RndContext& context) {
    constexpr unsigned int kAllStages = (1U << kNumShaderProgramTypes) - 1;
    context._DeselectAllSourceTexturesImpl(kAllStages);
    for (unsigned long& limit : context.mOutputSlotLimits) {
        limit = 0;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x62E1C0. Every field, the buffer size
// included, starts at -1.
RndCShaderBlurClassify::RndCShaderBlurClassify()
    : mSrcBuffer(-1),
      mSrcClassificationBuffer(-1),
      mDstClassificationBuffer(-1),
      mCBufferSize(-1),
      mDimensions(-1),
      mTileParams(-1) {}

// Reconstructed from eboot.elf at 0x62E200.
RndCShaderBlurClassify::~RndCShaderBlurClassify() {}

// Reconstructed from eboot.elf at 0x62E230.
void RndCShaderBlurClassify::Dispatch(
    RndContext& context,
    const Params& params) {
    context.SetRenderTargets(nullptr, nullptr);
    DeselectSourceTextures(context);
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Blur Classify");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
    params.mSrc->Select(context, kShaderProgramCompute, mSrcBuffer, 0, 0);
    params.mSrcClassification->Select(
        context, kShaderProgramCompute, mSrcClassificationBuffer, 0, 0);
    params.mDstClassification->Select(
        context,
        kShaderProgramCompute,
        mDstClassificationBuffer,
        RndShaderResource::kSelectReadWrite,
        0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto& src = params.mSrc->mBaseDesc;
    const auto& dst = params.mDstClassification->mBaseDesc;
    auto* dimensions = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mDimensions));
    dimensions[0] = static_cast<float>(static_cast<int>(src.mWidth));
    dimensions[1] = static_cast<float>(static_cast<int>(src.mHeight));
    auto* tileParams = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTileParams));
    tileParams[0] = static_cast<float>(static_cast<int>(dst.mWidth));
    tileParams[1] = static_cast<float>(static_cast<int>(dst.mHeight));
    tileParams[2] = static_cast<float>(params.mTileSize);
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    context._DispatchComputeImpl(dst.mWidth, dst.mHeight, 1);
    DeselectSourceTextures(context);
}

// Reconstructed from eboot.elf at 0x62E630.
const char* RndCShaderBlurClassify::_GetClassNameImpl() const {
    return "RndCShaderBlurClassify";
}

// Reconstructed from eboot.elf at 0x62E530.
const char* RndCShaderBlurClassify::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/BlurClassify.hlsl";
}

// Reconstructed from eboot.elf at 0x62E540.
void RndCShaderBlurClassify::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSrcBuffer = resources.AddTexture(
        "gSrcBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSrcClassificationBuffer = resources.AddTexture(
        "gSrcClassificationBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDstClassificationBuffer = resources.AddTextureWritable(
        "gDstClassificationBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat2, "gDimensions");
    mTileParams = cbuffer.AddConstant(kShaderNumericFloat3, "gTileParams");
    mCBufferSize = cbuffer.mSize;
}
