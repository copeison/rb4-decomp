#include "render/lighting/deferred/RndLightDeferredShader.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/MakeString.h"

namespace {

// Index of the pixel program's key. Name not in the reference map.
constexpr unsigned long kPixelKey = 3;
// Shading-mode values allowed for the pixel programs. Names not in the
// reference map.
constexpr unsigned int kStandardShadingMode = kShadingModeStandard;
constexpr unsigned int kDebugMiscShadingMode = kShadingModeDebugMisc;

}  // namespace

// Reconstructed from eboot.elf at 0x6DB320.
RndLightDeferredShader::RndLightDeferredShader()
    : mShadingMode{},
      mIllumType{},
      mGBuffers{static_cast<unsigned long>(-1),
                static_cast<unsigned long>(-1),
                static_cast<unsigned long>(-1)},
      mLinearDepthMap(-1),
      mFunctionTable(-1) {}

RndLightDeferredShader::~RndLightDeferredShader() {}

// Reconstructed from eboot.elf at 0x6DB3B0.
void RndLightDeferredShader::SelectCommon(
    RndContext& context,
    RndBufferCollection& buffers) {
    const auto& frame =
        buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
    RndTextureBase* const gbuffers[3] = {
        frame.mGBufferColor,
        frame.mGBufferPixelNormals,
        frame.mGBufferVertexNormals,
    };
    for (unsigned long i = 0; i < 3; ++i) {
        if (gbuffers[i] != nullptr) {
            gbuffers[i]->Select(context, kShaderProgramPixel, mGBuffers[i], 0, 0);
        }
    }
    frame.mLinearDepth->Select(
        context, kShaderProgramPixel, mLinearDepthMap, 0, 0);
    TheRndDevice()->mShaderMgr.mFunctionTable->Select(
        context, kShaderProgramPixel, mFunctionTable, 0, 0);
}

// Reconstructed from eboot.elf at 0x6DB590.
void RndLightDeferredShader::_SelectBase(
    RndContext& context,
    const Params& params,
    RndShaderKeyGroup& keys) {
    auto key = mShadingMode.SetValue(
        keys.mKeys[kPixelKey], static_cast<unsigned int>(context.mShadingMode));
    keys.mKeys[kPixelKey] =
        mIllumType.SetValue(key, static_cast<unsigned int>(params.mIllumType));
    _SelectShaderCollection(context, keys);
}

// Reconstructed from eboot.elf at 0x6DB5F0.
void RndLightDeferredShader::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig& resources) {
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mShadingMode = pixel.Add(Symbol("HX_SHADING_MODE"), 0, 19);
    mIllumType = pixel.Add(Symbol("HX_ILLUM_TYPE"), 0, 4);
    for (int i = 0; i < 3; ++i) {
        const char* name = MakeString("gGBuffer%d", i);
        const char* sampler = MakeString("gGBuffer%dSampler", i);
        mGBuffers[i] = resources.AddTexture(
            name,
            sampler,
            RndTextureBase::kTexture2D,
            kShaderProgramPixel,
            kShaderNumericFloat4);
    }
    mLinearDepthMap = resources.AddTexture(
        "gLinearDepthMap",
        "gLinearDepthMapSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mFunctionTable = resources.AddTexture(
        "gFunctionTable",
        "gFunctionTableSampler",
        RndTextureBase::kTextureArray1D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x6DB8F0. Pixel programs exist only for
// the standard and debug shading modes.
bool RndLightDeferredShader::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (type == kShaderProgramPixel) {
        const auto shadingMode = mShadingMode.GetValue(key);
        if (shadingMode != kDebugMiscShadingMode &&
            shadingMode != kStandardShadingMode) {
            return false;
        }
    }
    return true;
}
