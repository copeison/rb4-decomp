#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;

// Base of the deferred light shaders: the shading-mode and illumination-type
// permutations, and the G-buffer, linear depth and function-table inputs
// every deferred light reads. The vtable is at 0x1939278; _GetClassNameImpl
// and _GetShaderFilePath stay pure.
class RndLightDeferredShader : public RndShader {
public:
    // Selection options shared by every deferred light. The constructor
    // makes the type non-POD, so derived parameters reuse its tail padding as
    // the shadow-generation shaders' parameters do.
    struct Params {
        Params() {}

        int mIllumType;  // Name not in the reference map.
    };

    RndLightDeferredShader();             // 0x6DB320
    ~RndLightDeferredShader() override;   // 0x6DB380, 0x6DB390

    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6DB5F0
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // slot 7 at 0x6DB8F0

    // Binds the G-buffer, the linear depth and the function table for the
    // pixel stage.
    void SelectCommon(RndContext& context, RndBufferCollection& buffers);  // 0x6DB3B0
    // Adds the context's shading mode and the illumination type to the pixel
    // key and selects the programs.
    void _SelectBase(
        RndContext& context,
        const Params& params,
        RndShaderKeyGroup& keys);  // 0x6DB590

    // Field names are not in the reference map.
    RndShaderDefInfo mShadingMode;
    RndShaderDefInfo mIllumType;
    unsigned long mGBuffers[3];  // Resource indices.
    unsigned long mLinearDepthMap;
    unsigned long mFunctionTable;
};

static_assert(sizeof(RndLightDeferredShader::Params) == 4);
static_assert(offsetof(RndLightDeferredShader, mShadingMode) == 288);
static_assert(offsetof(RndLightDeferredShader, mIllumType) == 308);
static_assert(offsetof(RndLightDeferredShader, mGBuffers) == 328);
static_assert(offsetof(RndLightDeferredShader, mFunctionTable) == 360);
static_assert(sizeof(RndLightDeferredShader) == 368);
