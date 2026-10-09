#pragma once

#include <cstdint>

namespace rb4 {

// Primary-shader dispatch tables have 11 slots. Slots 0-6 are declared by each
// shader beside its implementation; these are the shared defaults for slots
// 7-10, recovered from the base table at 0x192EF60.

// Slot 7: whether a permutation key is valid for a shader stage.
bool render_primary_shader_validate_permutation(
    void* shader,
    std::uint32_t stage,
    std::uint64_t key);
// Slot 8: binds a fallback program on a render context.
void render_primary_shader_bind_fallback(void* shader, void* context);
// Slot 9: whether six-slice render-target permutations are supported.
bool render_primary_shader_supports_render_target_slices(void* shader);
// Slot 10: whether single-slice permutations include a geometry program.
bool render_primary_shader_uses_geometry_program(void* shader);

// Overrides shared by several shaders.
bool render_primary_shader_returns_true(void* shader);
void render_primary_shader_bind_nothing(void* shader, void* context);

}  // namespace rb4
