#pragma once

#include "render/postprocessing/tonemap/RndTonemapShaderBase.h"

class RndContext;

// Tonemaps the scene in a compute shader, one thread per output pixel. The
// vtable is at 0x1939210.
class RndCShaderTonemap : public RndTonemapShaderBase {
public:
    RndCShaderTonemap();             // 0x6DAF60
    ~RndCShaderTonemap() override;   // 0x6DB000, 0x6DB010

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6DB030
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6DAFF0
    int _GetShaderStages() const override;           // slot 6 at 0x6DB040

    // Selects the shader and dispatches 8x8 groups over the output. The
    // map's signature is Dispatch(RndContext&, RndTextureBase&,
    // RndTextureBase&, float).
    void Dispatch(RndContext& context, const Params& params);  // 0x6DAF90
};

static_assert(sizeof(RndCShaderTonemap) == 368);
