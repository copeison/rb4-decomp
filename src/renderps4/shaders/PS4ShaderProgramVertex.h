#pragma once

#include <cstddef>

#include "render/shaders/RndShaderProgram.h"

// PS4 vertex shader program. The vtable is at 0x195F920.
class PS4ShaderProgramVertex : public RndShaderProgram {
public:
    PS4ShaderProgramVertex();            // 0x8E46E0
    ~PS4ShaderProgramVertex() override;  // 0x8E4720, 0x8E4750

    // Slots 2-4 are not yet reconstructed.
    bool _CreateImpl(BinStream& stream) override;  // 0x8E4790
    void _SelectImpl(RndContext& context) override;  // 0x8E4D80
    void _FreeImpl() override;                       // 0x8E4ED0
    RndShaderProgramType _GetTypeImpl() const override;  // 0x8E4F30

    // Backend program state; its layout has not been recovered.
    unsigned char mBackend[64];

private:
    // Stand-in for the backend defaults inlined into the constructor; not yet
    // reconstructed. Name not in the reference map.
    void _InitBackend();
};

static_assert(sizeof(PS4ShaderProgramVertex) == 104);
