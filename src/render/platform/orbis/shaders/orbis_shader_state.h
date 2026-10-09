#pragma once

#include <cstdint>

#include "render/shaders/RndShaderProgram.h"

class PS4Context;

namespace rb4 {

enum class OrbisSamplerAddressMode : std::uint32_t {
    kClamp = 1,
    kWrap = 2,
    kClampOpaqueBlack = 3,
    kClampOpaqueWhite = 4,
    kMirror = 5,
};

struct OrbisSamplerDescriptor {
    std::uint32_t registers[4];
};

}  // namespace rb4
