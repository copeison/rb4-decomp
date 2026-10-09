#include "render/video/RndShaderBinkConvert.h"

#include <cstring>

#include "render/buffers/RndShaderCBuffer.h"
#include "render/shaders/RndShaderDrawUtl.h"

#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x5F4980. The four padding bytes after the
// define are left untouched.
RndShaderBinkConvert::RndShaderBinkConvert()
    : mBinkAlphaPlane{},
      mYPlane(-1),
      mCRPlane(-1),
      mCBPlane(-1),
      mAPlane(-1),
      mYScale(-1),
      mCRScale(-1),
      mCBScale(-1),
      mFullScale(-1),
      mFullOffset(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x5F49E0.
RndShaderBinkConvert::~RndShaderBinkConvert() {}

// Reconstructed from eboot.elf at 0x5F4A10. The luma and chroma planes are
// always bound; the alpha plane is optional and selects the alpha-plane
// permutation on the pixel program.
void RndShaderBinkConvert::Select(RndContext& context, const Params& params) {
    constexpr unsigned long kPixelKey = 3;

    params.mYPlane->Select(context, kShaderProgramPixel, mYPlane, 0, 0);
    params.mCRPlane->Select(context, kShaderProgramPixel, mCRPlane, 0, 0);
    params.mCBPlane->Select(context, kShaderProgramPixel, mCBPlane, 0, 0);
    RndShaderDrawUtl::SelectPixelTexture(context, params.mAPlane, mAPlane);

    auto& buffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mYScale),
        params.mYScale,
        sizeof(params.mYScale));
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mCRScale),
        params.mCRScale,
        sizeof(params.mCRScale));
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mCBScale),
        params.mCBScale,
        sizeof(params.mCBScale));
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mFullScale),
        params.mFullScale,
        sizeof(params.mFullScale));
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mFullOffset),
        params.mFullOffset,
        sizeof(params.mFullOffset));
    RndShaderDrawUtl::CommitCBuffer(buffer, context, mCBufferSize);

    RndShaderKeyGroup keys{};
    keys.mKeys[kPixelKey] =
        mBinkAlphaPlane.SetValue(0, params.mAPlane != nullptr ? 1U : 0U);
    _SelectShaderCollection(context, keys);
}

// Reconstructed from eboot.elf at 0x5F4E70.
const char* RndShaderBinkConvert::_GetClassNameImpl() const {
    return "RndShaderBinkConvert";
}

// Reconstructed from eboot.elf at 0x5F4C90.
const char* RndShaderBinkConvert::_GetShaderFilePath() const {
    return "../../system/data/shaders/BinkConvert.hlsl";
}

// Reconstructed from eboot.elf at 0x5F4CA0.
void RndShaderBinkConvert::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mBinkAlphaPlane = defines.GetDefines(kShaderProgramPixel)
                          .AddBool(Symbol("HX_BINK_ALPHA_PLANE"));

    mYPlane = resources.AddTexture(
        "gYPlane",
        "gYPlaneSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mCRPlane = resources.AddTexture(
        "gCRPlane",
        "gCRPlaneSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mCBPlane = resources.AddTexture(
        "gCBPlane",
        "gCBPlaneSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mAPlane = resources.AddTexture(
        "gAPlane",
        "gAPlaneSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);

    mYScale = cbuffer.AddConstant(kShaderNumericFloat4, "gYScale");
    mCRScale = cbuffer.AddConstant(kShaderNumericFloat4, "gCRScale");
    mCBScale = cbuffer.AddConstant(kShaderNumericFloat4, "gCBScale");
    mFullScale = cbuffer.AddConstant(kShaderNumericFloat4, "gFullScale");
    mFullOffset = cbuffer.AddConstant(kShaderNumericFloat4, "gFullOffset");
    mCBufferSize = cbuffer.mSize;
}
