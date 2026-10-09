#include "render/lighting/deferred/RndLightProbeDeferredAccumShader.h"

#include <cstring>

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x49DBA0.
RndLightProbeDeferredAccumShader::RndLightProbeDeferredAccumShader()
    : mProbeAccumBuffer(-1), mCBufferSize(0), mProbeIntensityMult(-1) {}

RndLightProbeDeferredAccumShader::~RndLightProbeDeferredAccumShader() {}

// Reconstructed from eboot.elf at 0x49DC20.
void RndLightProbeDeferredAccumShader::Select(
    RndContext& context,
    Params& params) {
    params.mProbeAccumBuffer->Select(
        context, kShaderProgramPixel, mProbeAccumBuffer, 0, 0);
    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mProbeIntensityMult),
        &params.mProbeIntensityMult,
        sizeof(params.mProbeIntensityMult));
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);
    RndShaderKeyGroup keys{};
    _SelectBase(context, params, keys);
}

const char* RndLightProbeDeferredAccumShader::_GetShaderFilePath() const {
    return "../../system/data/shaders/LightProbeDeferredAccum.hlsl";
}

// Reconstructed from eboot.elf at 0x49DD50.
void RndLightProbeDeferredAccumShader::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    RndLightDeferredShader::_InitConfigImpl(
        fixedDefines, defines, cbuffer, resources);
    mProbeAccumBuffer = resources.AddTexture(
        "gProbeAccumBuffer",
        "gProbeAccumSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mProbeIntensityMult =
        cbuffer.AddConstant(kShaderNumericFloat, "gProbeIntensityMult");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x49DDD0. The accumulation uses only the
// first illumination type.
bool RndLightProbeDeferredAccumShader::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (!RndLightDeferredShader::_UsesShaderKeyImpl(type, key)) {
        return false;
    }
    return type != kShaderProgramPixel || mIllumType.GetValue(key) == 0;
}

const char* RndLightProbeDeferredAccumShader::_GetClassNameImpl() const {
    return "RndLightProbeDeferredAccumShader";
}
