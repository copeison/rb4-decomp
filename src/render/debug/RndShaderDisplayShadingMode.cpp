#include "render/debug/RndShaderDisplayShadingMode.h"

#include "render/context/RndContext.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x63DFB0 and never read. Names not in the reference map.
[[maybe_unused]] int gShadingModeGroupSizeUnused = -1;  // 0x1AAB770
[[maybe_unused]] int gShadingModeGroupSize2D = 8;       // 0x1AAB774
[[maybe_unused]] int gShadingModeGroupSize3D = 4;       // 0x1AAB778

// The overdraw limits convert as unsigned counts. Name not in the reference
// map.
float OverdrawLimit(std::int64_t limit) {
    return static_cast<float>(static_cast<unsigned long>(limit));
}

}  // namespace

// Reconstructed from eboot.elf at 0x63DC90.
RndShaderDisplayShadingMode::RndShaderDisplayShadingMode()
    : mMaxOverdraw(-1), mCBufferSize(0), mSceneTex(-1) {}

// Reconstructed from eboot.elf at 0x63DCE0.
RndShaderDisplayShadingMode::~RndShaderDisplayShadingMode() {}

// Reconstructed from eboot.elf at 0x63DD10.
void RndShaderDisplayShadingMode::Select(
    RndContext& context,
    RndBufferCollection& buffers,
    const RndSceneDrawTarget& target) {
    const RndConfig* const settings = TheRndDevice()->mSettings;
    const float geoOverdraw = OverdrawLimit(settings->mMaxGeoOverdraw);
    const float lightingOverdraw =
        OverdrawLimit(settings->mMaxLightingOverdraw);
    const float lightProbeOverdraw =
        OverdrawLimit(settings->mMaxLightProbeOverdraw);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* maxOverdraw = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mMaxOverdraw));
    maxOverdraw[0] = geoOverdraw;
    maxOverdraw[1] = lightingOverdraw;
    maxOverdraw[2] = lightProbeOverdraw;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    RndTextureBase* const scene = buffers.mActiveSceneContext != 0
        ? buffers.mLightAccum[target.mSrcLightAccum]
        : buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval]
              .mPartialLightAccum;
    scene->Select(context, kShaderProgramPixel, mSceneTex, 0, 0);

    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
}

// Reconstructed from eboot.elf at 0x63DF10.
const char* RndShaderDisplayShadingMode::_GetShaderFilePath() const {
    return "../../system/data/shaders/DisplayShadingMode.hlsl";
}

// Reconstructed from eboot.elf at 0x63DF20.
void RndShaderDisplayShadingMode::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mMaxOverdraw = cbuffer.AddConstant(kShaderNumericFloat3, "gMaxOverdraw");
    mCBufferSize = cbuffer.mSize;
    mSceneTex = resources.AddTexture(
        "gSceneTex",
        "gSceneTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x63DFA0.
const char* RndShaderDisplayShadingMode::_GetClassNameImpl() const {
    return "RndShaderDisplayShadingMode";
}
