#pragma once

#include <cstddef>

#include "render/shaders/RndShaderProgram.h"

// PS4 geometry shader program. The vtable is at 0x195F8A0.
class PS4ShaderProgramGeometry : public RndShaderProgram {
public:
    PS4ShaderProgramGeometry();            // 0x8E40A0
    ~PS4ShaderProgramGeometry() override;  // 0x8E40D0, 0x8E4100

    // Slots 2-4 are not yet reconstructed.
    bool _CreateImpl(BinStream& stream) override;  // 0x8E4140
    void _SelectImpl(RndContext& context) override;  // 0x8E4330
    void _FreeImpl() override;                       // 0x8E4350
    RndShaderProgramType _GetTypeImpl() const override;  // 0x8E43A0

    // Backend program state; its layout has not been recovered.
    unsigned char mBackend[32];

private:
    // Stand-in for the backend defaults inlined into the constructor; not yet
    // reconstructed. Name not in the reference map.
    void _InitBackend();
};

static_assert(sizeof(PS4ShaderProgramGeometry) == 72);
