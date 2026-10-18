#include "render/distance_fields/RndCShaderSignedDistanceClassify.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x62EEF0; none is used. Names not in the reference map.
[[maybe_unused]] int gClassifyGroupSizeUnused = -1;  // 0x1AAA778
[[maybe_unused]] int gClassifyGroupSize2D = 8;       // 0x1AAA77C
[[maybe_unused]] int gClassifyGroupSize3D = 4;       // 0x1AAA780

}  // namespace

// Reconstructed from eboot.elf at 0x62EAF0. Every field, the buffer size
// included, starts at -1.
RndCShaderSignedDistanceClassify::RndCShaderSignedDistanceClassify()
    : mSrcBuffer(-1),
      mDstBuffer(-1),
      mDstClassificationBuffer(-1),
      mCBufferSize(-1),
      mDimensions(-1),
      mTileParams(-1) {}

// Reconstructed from eboot.elf at 0x62EB30.
RndCShaderSignedDistanceClassify::~RndCShaderSignedDistanceClassify() {}

// Reconstructed from eboot.elf at 0x62EB60. One thread per tile.
void RndCShaderSignedDistanceClassify::Dispatch(
    RndContext& context,
    const Params& params) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Signed Distance Classify");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
    params.mSrc->Select(context, kShaderProgramCompute, mSrcBuffer, 0, 0);
    params.mDst->Select(
        context,
        kShaderProgramCompute,
        mDstBuffer,
        RndShaderResource::kSelectReadWrite,
        0);
    params.mDstClassification->Select(
        context,
        kShaderProgramCompute,
        mDstClassificationBuffer,
        RndShaderResource::kSelectReadWrite,
        0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto& src = params.mSrc->mBaseDesc;
    const auto& tiles = params.mDstClassification->mBaseDesc;
    auto* dimensions = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mDimensions));
    dimensions[0] = static_cast<float>(static_cast<int>(src.mWidth));
    dimensions[1] = static_cast<float>(static_cast<int>(src.mHeight));
    auto* tileParams = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTileParams));
    tileParams[0] = static_cast<float>(static_cast<int>(tiles.mWidth));
    tileParams[1] = static_cast<float>(static_cast<int>(tiles.mHeight));
    tileParams[2] = static_cast<float>(params.mTileSize);
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    context._DispatchComputeImpl(tiles.mWidth, tiles.mHeight, 1);
}

// Reconstructed from eboot.elf at 0x62EDF0.
const char* RndCShaderSignedDistanceClassify::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/SignedDistanceClassify.hlsl";
}

// Reconstructed from eboot.elf at 0x62EE00.
void RndCShaderSignedDistanceClassify::_InitConfigImpl(
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
    mDstBuffer = resources.AddTextureWritable(
        "gDstBuffer",
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

// Reconstructed from eboot.elf at 0x62EEE0.
const char* RndCShaderSignedDistanceClassify::_GetClassNameImpl() const {
    return "RndCShaderSignedDistanceClassify";
}
