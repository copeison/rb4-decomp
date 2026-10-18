#include "render/depth/RndCShaderCalcDepthRange.h"

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
// initializer at 0x6371F0; none is used. Names not in the reference map.
[[maybe_unused]] int gCalcDepthRangeGroupSizeUnused = -1;  // 0x1AAB6C0
[[maybe_unused]] int gCalcDepthRangeGroupSize2D = 8;       // 0x1AAB6C4
[[maybe_unused]] int gCalcDepthRangeGroupSize3D = 4;       // 0x1AAB6C8

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

}  // namespace

// Reconstructed from eboot.elf at 0x636DE0.
RndCShaderCalcDepthRange::RndCShaderCalcDepthRange()
    : mLinearDepthBuffer(-1),
      mTiledDepthRangeBuffer(-1),
      mTileCounts(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x636E30. One thread group per light
// tile.
void RndCShaderCalcDepthRange::Dispatch(
    RndContext& context,
    const Params& params) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Calc Depth Range");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    const RndConfig* const settings = TheRndDevice()->mSettings;
    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);

    const int tileSize = static_cast<int>(settings->mLightTileSize);
    const auto& depth = params.mLinearDepth->mBaseDesc;
    const int tilesX = CeilDiv(static_cast<int>(depth.mWidth), tileSize);
    const int tilesY = CeilDiv(static_cast<int>(depth.mHeight), tileSize);
    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* tileCounts = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTileCounts));
    tileCounts[0] = static_cast<float>(tilesX);
    tileCounts[1] = static_cast<float>(tilesY);
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    params.mLinearDepth->Select(
        context, kShaderProgramCompute, mLinearDepthBuffer, 0, 0);
    params.mTiledDepthRange->Select(
        context,
        kShaderProgramCompute,
        mTiledDepthRangeBuffer,
        RndShaderResource::kSelectReadWrite,
        0);
    context._DispatchComputeImpl(
        static_cast<unsigned int>(tilesX),
        static_cast<unsigned int>(tilesY),
        1);
}

// Reconstructed from eboot.elf at 0x6370A0.
const char* RndCShaderCalcDepthRange::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CalcDepthRange.hlsl";
}

// Reconstructed from eboot.elf at 0x6370B0. Reads the device's settings
// without a null check.
void RndCShaderCalcDepthRange::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    const auto& settings = *TheRndDevice()->mSettings;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"), static_cast<int>(settings.mLightTileSize));

    mLinearDepthBuffer = resources.AddTexture(
        "gLinearDepthBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTiledDepthRangeBuffer = resources.AddTextureWritable(
        "gTiledDepthRangeBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTileCounts = cbuffer.AddConstant(kShaderNumericFloat2, "gTileCounts");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x6371B0.
RndCShaderCalcDepthRange::~RndCShaderCalcDepthRange() {}

// Reconstructed from eboot.elf at 0x6371E0.
const char* RndCShaderCalcDepthRange::_GetClassNameImpl() const {
    return "RndCShaderCalcDepthRange";
}
