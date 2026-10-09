#pragma once

#include <cstddef>

#include "render/shaders/RndShaderProgram.h"

// PS4 compute shader program. The vtable is at 0x195F860.
class PS4ShaderProgramCompute : public RndShaderProgram {
public:
    PS4ShaderProgramCompute();            // 0x8E3D20
    ~PS4ShaderProgramCompute() override;  // 0x8E3D50, 0x8E3D80

    // Slots 2-4 are not yet reconstructed.
    bool _CreateImpl(BinStream& stream) override;  // 0x8E3DC0
    void _SelectImpl(RndContext& context) override;  // 0x8E3F40
    void _FreeImpl() override;                       // 0x8E4030
    RndShaderProgramType _GetTypeImpl() const override;  // 0x8E4070

    // Backend program state; its layout has not been recovered.
    unsigned char mBackend[24];

private:
    // Stand-in for the backend defaults inlined into the constructor; not yet
    // reconstructed. Name not in the reference map.
    void _InitBackend();
};

static_assert(sizeof(PS4ShaderProgramCompute) == 64);
