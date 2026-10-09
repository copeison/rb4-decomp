#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisRenderCommandContext;

enum class OrbisIndexSize : std::uint32_t {
    k16Bit = 0,
    k32Bit = 1,
};

enum class MeshPrimitiveType : std::uint32_t {
    kTriangles = 3,
};

}  // namespace rb4
