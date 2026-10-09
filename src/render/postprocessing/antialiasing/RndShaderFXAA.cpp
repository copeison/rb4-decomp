#include "render/postprocessing/antialiasing/RndShaderFXAA.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6364F0.
RndShaderFXAA::RndShaderFXAA()
    : mTargetDimensionsRcp(-1), mCBufferSize(0), mSrcTex(-1) {}

// Reconstructed from eboot.elf at 0x636540.
RndShaderFXAA::~RndShaderFXAA() {}

// Reconstructed from eboot.elf at 0x636770.
const char* RndShaderFXAA::_GetClassNameImpl() const {
    return "RndShaderFXAA";
}

// Reconstructed from eboot.elf at 0x6366F0.
const char* RndShaderFXAA::_GetShaderFilePath() const {
    return "../../system/data/shaders/FXAA.hlsl";
}

// Reconstructed from eboot.elf at 0x636700.
void RndShaderFXAA::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSrcTex = resources.AddTexture(
        "gSrcTex",
        "gSrcTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTargetDimensionsRcp =
        cbuffer.AddConstant(kShaderNumericFloat2, "gTargetDimensionsRcp");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x636570. The source is bound with
// bilinear filtering whatever its own filter mode; without a source the
// reciprocal falls back to a 1600x900 target.
void RndShaderFXAA::Select(RndContext& context, const Params& params) {
    // The filter-mode value is inferred from this use.
    constexpr unsigned int kFilterBilinear = 2;
    constexpr float kDefaultWidth = 1600.0F;
    constexpr float kDefaultHeight = 900.0F;

    auto width = kDefaultWidth;
    auto height = kDefaultHeight;
    if (auto* source = params.mSource) {
        auto& filterMode = source->mBaseDesc.mFormat.mFilterMode;
        const auto savedFilterMode = filterMode;
        filterMode = kFilterBilinear;
        source->Select(context, kShaderProgramPixel, mSrcTex, 0, 0);
        width = static_cast<float>(static_cast<int>(source->mBaseDesc.mWidth));
        height =
            static_cast<float>(static_cast<int>(source->mBaseDesc.mHeight));
        filterMode = savedFilterMode;
    }

    auto& buffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* dimensionsRcp = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(buffer, mTargetDimensionsRcp));
    dimensionsRcp[0] = 1.0F / width;
    dimensionsRcp[1] = 1.0F / height;
    RndShaderDrawUtl::CommitCBuffer(buffer, context, mCBufferSize);

    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
}
