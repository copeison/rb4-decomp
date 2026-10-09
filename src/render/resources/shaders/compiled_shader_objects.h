#pragma once

#include <cstddef>
#include <cstdint>

class BinStream;
class RndShaderProgram;

class RndContext;

namespace rb4 {

struct RenderManagedObjectArray {
    RndShaderProgram** begin;
    RndShaderProgram** end;
    RndShaderProgram** capacity;
    void* allocator;
};

static_assert(sizeof(RenderManagedObjectArray) == 32);

// The primary shader keeps one compiled-object vector per shader stage,
// indexed by RndShaderProgramType.
constexpr std::size_t kRenderShaderStageCount = 6;
// Permutation keys are kept per variant program bit; hull and domain share
// one key.
constexpr std::size_t kRenderShaderProgramKeyCount = 5;

void render_compiled_shader_objects_resize(
    RenderManagedObjectArray& objects,
    std::size_t count);
bool render_compiled_shader_objects_load(
    RenderManagedObjectArray (&objects)[kRenderShaderStageCount],
    const char* name,
    BinStream& stream);

bool render_compiled_shader_objects_bind(
    RenderManagedObjectArray (&objects)[kRenderShaderStageCount],
    RndContext& context,
    std::int32_t variant,
    const std::uint64_t (&keys)[kRenderShaderProgramKeyCount]);

}  // namespace rb4
