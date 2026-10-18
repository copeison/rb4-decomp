#include "render/masking/RndShaderStencilSceneMask.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x6452C0 and never read. Names not in the reference map.
[[maybe_unused]] int gStencilSceneMaskGroupSizeUnused = -1;  // 0x1AAB814
[[maybe_unused]] int gStencilSceneMaskGroupSize2D = 8;       // 0x1AAB818
[[maybe_unused]] int gStencilSceneMaskGroupSize3D = 4;       // 0x1AAB81C

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

}  // namespace

// Reconstructed from eboot.elf at 0x644FC0.
RndShaderStencilSceneMask::RndShaderStencilSceneMask()
    : mUnrefinedMask(-1), mTileCounts(-1), mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x645000.
RndShaderStencilSceneMask::~RndShaderStencilSceneMask() {}

// Reconstructed from eboot.elf at 0x645030. The tile counts cover the
// context's viewport.
void RndShaderStencilSceneMask::Select(
    RndContext& context,
    RndTextureBase& unrefinedMask) {
    unrefinedMask.Select(context, kShaderProgramVertex, mUnrefinedMask, 0, 0);

    const int tileSize =
        static_cast<int>(TheRndDevice()->mSettings->mLightTileSize);
    const auto width = static_cast<int>(context.mViewportSize.x);
    const auto height = static_cast<int>(context.mViewportSize.y);
    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* tileCounts = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTileCounts));
    tileCounts[0] = static_cast<float>(CeilDiv(width, tileSize));
    tileCounts[1] = static_cast<float>(CeilDiv(height, tileSize));
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
}

// Reconstructed from eboot.elf at 0x6451B0.
const char* RndShaderStencilSceneMask::_GetShaderFilePath() const {
    return "../../system/data/shaders/StencilSceneMask.hlsl";
}

// Reconstructed from eboot.elf at 0x6451C0.
void RndShaderStencilSceneMask::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto* device = TheRndDevice();
    const auto* settings = device != nullptr ? device->mSettings : nullptr;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"),
        static_cast<int>(
            settings != nullptr ? settings->mLightTileSize
                                : RndConfig::kDefaultLightTileSize));

    mUnrefinedMask = resources.AddTexture(
        "gUnrefinedMask",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramVertex,
        kShaderNumericFloat4);
    mTileCounts = cbuffer.AddConstant(kShaderNumericFloat2, "gTileCounts");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x6452A0. Only the vertex program.
int RndShaderStencilSceneMask::_GetShaderStages() const {
    return 1;
}

// Reconstructed from eboot.elf at 0x6452B0.
const char* RndShaderStencilSceneMask::_GetClassNameImpl() const {
    return "RndShaderStencilSceneMask";
}
