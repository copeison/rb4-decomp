#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Fallback shader selected when another shader's programs are missing. The
// vtable is at 0x192F2A0.
class RndShaderError : public RndShader {
public:
    RndShaderError();             // 0x63E650
    ~RndShaderError() override;   // 0x63E690, 0x63E6A0

    const char* _GetClassNameImpl() const override;   // 0x63E830
    const char* _GetShaderFilePath() const override;  // 0x63E730
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x63E740
    // The error shader has no fallback of its own.
    void _SelectErrorShader(RndContext& context) const override;  // 0x63E820
    bool _SupportsRTSlicing() const override;                    // 0x63E810

    // Selects the error programs for the geometry type and the context's
    // shading mode.
    void Select(RndContext& context, RndShaderGeoType geoType);  // 0x63E6C0

    // Field names are not in the reference map.
    RndShaderDefInfo mGeoType;
    RndShaderDefInfo mShadingMode;
};

static_assert(offsetof(RndShaderError, mGeoType) == 288);
static_assert(sizeof(RndShaderError) == 328);
