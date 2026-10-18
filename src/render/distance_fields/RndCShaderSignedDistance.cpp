#include "render/distance_fields/RndCShaderSignedDistance.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x62EAD0; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gSignedDistanceGroupSizeUnused = -1;  // 0x1AAA758
int gSignedDistanceGroupSize2D = 8;                        // 0x1AAA75C
[[maybe_unused]] int gSignedDistanceGroupSize3D = 4;       // 0x1AAA760

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

}  // namespace

// Reconstructed from eboot.elf at 0x62E660. Every field, the buffer size
// included, starts at -1.
RndCShaderSignedDistance::RndCShaderSignedDistance()
    : mSrcBuffer(-1),
      mClassificationBuffer(-1),
      mDstBuffer(-1),
      mCBufferSize(-1),
      mDimensions(-1),
      mTileParams(-1),
      mDistanceParams(-1) {}

// Reconstructed from eboot.elf at 0x62E6A0.
RndCShaderSignedDistance::~RndCShaderSignedDistance() {}

// Reconstructed from eboot.elf at 0x62E6D0.
void RndCShaderSignedDistance::Dispatch(
    RndContext& context,
    const Params& params) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Signed Distance");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
    params.mSrc->Select(context, kShaderProgramCompute, mSrcBuffer, 0, 0);
    params.mClassification->Select(
        context, kShaderProgramCompute, mClassificationBuffer, 0, 0);
    params.mDst->Select(
        context,
        kShaderProgramCompute,
        mDstBuffer,
        RndShaderResource::kSelectReadWrite,
        0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto member = [&cbuffer](unsigned long offset) {
        return static_cast<float*>(
            RndShaderDrawUtl::GetCBufferMember(cbuffer, offset));
    };
    const auto& src = params.mSrc->mBaseDesc;
    const auto& tiles = params.mClassification->mBaseDesc;
    float* const dimensions = member(mDimensions);
    dimensions[0] = static_cast<float>(static_cast<int>(src.mWidth));
    dimensions[1] = static_cast<float>(static_cast<int>(src.mHeight));
    float* const tileParams = member(mTileParams);
    tileParams[0] = static_cast<float>(static_cast<int>(tiles.mWidth));
    tileParams[1] = static_cast<float>(static_cast<int>(tiles.mHeight));
    tileParams[2] = static_cast<float>(params.mTileSize);
    const auto maxDistance = static_cast<float>(params.mMaxDistance);
    float* const distance = member(mDistanceParams);
    distance[0] = maxDistance;
    distance[1] = 1.0F / maxDistance;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(src.mWidth), gSignedDistanceGroupSize2D)),
        static_cast<unsigned int>(CeilDiv(
            static_cast<int>(src.mHeight), gSignedDistanceGroupSize2D)),
        1);
}

// Reconstructed from eboot.elf at 0x62E9B0.
const char* RndCShaderSignedDistance::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/SignedDistance.hlsl";
}

// Reconstructed from eboot.elf at 0x62E9C0.
void RndCShaderSignedDistance::_InitConfigImpl(
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
    mClassificationBuffer = resources.AddTexture(
        "gClassificationBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDstBuffer = resources.AddTextureWritable(
        "gDstBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat2, "gDimensions");
    mTileParams = cbuffer.AddConstant(kShaderNumericFloat3, "gTileParams");
    mDistanceParams =
        cbuffer.AddConstant(kShaderNumericFloat2, "gDistanceParams");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x62EAC0.
const char* RndCShaderSignedDistance::_GetClassNameImpl() const {
    return "RndCShaderSignedDistance";
}
