#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderShaderParameterRegistrySet;

// One preprocessor define selected by a permutation: the parameter name and
// the interned decimal text of its value.
struct RenderShaderPermutationDefine {
    const char* name;
    const char* value;
};

static_assert(sizeof(RenderShaderPermutationDefine) == 16);

// Invoked once per complete permutation with the selected defines in
// parameter order and the packed 64-bit permutation key.
using RenderShaderPermutationVisitor = void (*)(
    void* context,
    const RenderShaderPermutationDefine* defines,
    std::size_t define_count,
    std::uint64_t key);

bool render_shader_stage_enabled(std::int32_t variant, std::uint32_t stage);
void render_shader_enumerate_stage_permutations(
    const RenderShaderParameterRegistrySet& parameters,
    std::uint32_t stage,
    RenderShaderPermutationVisitor visitor,
    void* context);

}  // namespace rb4
