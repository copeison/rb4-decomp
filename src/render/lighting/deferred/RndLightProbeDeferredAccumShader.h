#pragma once

#include <cstddef>

#include "render/lighting/deferred/RndLightDeferredShader.h"

class RndContext;
class RndTextureBase;

// Adds the accumulated light probes, scaled by an intensity, to the light
// buffer. The vtable is at 0x1907518.
class RndLightProbeDeferredAccumShader : public RndLightDeferredShader {
public:
    // Field names are not in the reference map.
    struct Params : RndLightDeferredShader::Params {
        RndTextureBase* mProbeAccumBuffer;
        float mProbeIntensityMult;
    };

    RndLightProbeDeferredAccumShader();             // 0x49DBA0
    ~RndLightProbeDeferredAccumShader() override;   // 0x49DBF0, 0x49DC00

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x49DE40
    const char* _GetShaderFilePath() const override; // slot 3 at 0x49DD40
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x49DD50
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // slot 7 at 0x49DDD0

    // Binds the probe buffer, commits the intensity and selects the
    // programs.
    void Select(RndContext& context, Params& params);  // 0x49DC20

    // Field names are not in the reference map.
    unsigned long mProbeAccumBuffer;    // Resource index.
    unsigned long mCBufferSize;
    unsigned long mProbeIntensityMult;  // Constant offset.
};

static_assert(
    offsetof(RndLightProbeDeferredAccumShader::Params, mProbeAccumBuffer) == 8);
static_assert(
    offsetof(RndLightProbeDeferredAccumShader::Params, mProbeIntensityMult) ==
    16);
static_assert(
    offsetof(RndLightProbeDeferredAccumShader, mProbeAccumBuffer) == 368);
static_assert(sizeof(RndLightProbeDeferredAccumShader) == 392);
