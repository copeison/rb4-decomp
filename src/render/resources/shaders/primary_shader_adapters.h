#pragma once

#include <cstdint>

namespace rb4 {

// Error-shader permutation bind at 0x63E6C0. Selects the geometry type and the
// context's shading mode, then binds through 0x638920.
void render_error_shader_bind(
    void* shader,
    void* context,
    std::int32_t geometry_type);

}  // namespace rb4
