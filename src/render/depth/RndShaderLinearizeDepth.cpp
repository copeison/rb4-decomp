#include "render/depth/RndShaderLinearizeDepth.h"

#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x63F0F0 and never read. Names not in the reference map.
[[maybe_unused]] int gLinearizeDepthGroupSizeUnused = -1;  // 0x1AAB7AC
[[maybe_unused]] int gLinearizeDepthGroupSize2D = 8;       // 0x1AAB7B0
[[maybe_unused]] int gLinearizeDepthGroupSize3D = 4;       // 0x1AAB7B4

}  // namespace

// Reconstructed from eboot.elf at 0x63EF70.
RndShaderLinearizeDepth::RndShaderLinearizeDepth() : mTexture(-1) {}

// Reconstructed from eboot.elf at 0x63EFA0.
RndShaderLinearizeDepth::~RndShaderLinearizeDepth() {}

// Reconstructed from eboot.elf at 0x63F0E0.
const char* RndShaderLinearizeDepth::_GetClassNameImpl() const {
    return "RndShaderLinearizeDepth";
}

// Reconstructed from eboot.elf at 0x63F080.
const char* RndShaderLinearizeDepth::_GetShaderFilePath() const {
    return "../../system/data/shaders/LinearizeDepth.hlsl";
}

// Reconstructed from eboot.elf at 0x63F090.
void RndShaderLinearizeDepth::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig& resources) {
    mTexture = resources.AddTexture(
        "gTexture",
        "gTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x63EFD0.
void RndShaderLinearizeDepth::Select(
    RndContext& context,
    RndTextureBase& depth) {
    RndShaderDrawUtl::SelectWithPixelTexture(*this, context, depth, mTexture);
}
