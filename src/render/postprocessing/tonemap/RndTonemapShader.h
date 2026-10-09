#pragma once

#include "render/postprocessing/tonemap/RndTonemapShaderBase.h"

class RndContext;

// Tonemaps the scene in a pixel shader. Not in the reference map. The
// vtable is at 0x19083D8.
class RndTonemapShader : public RndTonemapShaderBase {
public:
    RndTonemapShader();             // 0x4ADB80
    ~RndTonemapShader() override;   // 0x4ADBD0, 0x4ADBE0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x4ADC00
    const char* _GetShaderFilePath() const override; // slot 3 at 0x4ADBC0
    // Vertex and pixel programs.
    int _GetShaderStages() const override;           // slot 6 at 0x4ADC10

    // Name not in the reference map.
    void Select(RndContext& context, const Params& params);  // 0x4ADBB0
};

static_assert(sizeof(RndTonemapShader) == 368);
