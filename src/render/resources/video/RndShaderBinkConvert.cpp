#include "render/resources/video/RndShaderBinkConvert.h"

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
