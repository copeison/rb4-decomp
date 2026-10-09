#pragma once

#include <cstddef>

#include "render/shaders/RndShaderProgram.h"

// PS4 pixel shader program. The vtable is at 0x195F8E0.
class PS4ShaderProgramPixel : public RndShaderProgram {
public:
    PS4ShaderProgramPixel();            // 0x8E43D0
    ~PS4ShaderProgramPixel() override;  // 0x8E4410, 0x8E4440

    // Slots 2-4 are not yet reconstructed.
    bool _CreateImpl(BinStream& stream) override;  // 0x8E4480
    void _SelectImpl(RndContext& context) override;  // 0x8E4600
    void _FreeImpl() override;                       // 0x8E4670
    RndShaderProgramType _GetTypeImpl() const override;  // 0x8E46B0

    // Backend program state; its layout has not been recovered.
    unsigned char mBackend[24];

private:
    // Stand-in for the backend defaults inlined into the constructor; not yet
    // reconstructed. Name not in the reference map.
    void _InitBackend();
};

static_assert(sizeof(PS4ShaderProgramPixel) == 64);
