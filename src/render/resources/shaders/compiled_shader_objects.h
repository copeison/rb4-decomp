#pragma once

#include <cstddef>
#include <cstdint>

class BinStream;

namespace rb4 {

struct RenderContext;

struct RenderManagedObjectDispatch {
    void* reserved_0;
    void (*release_dynamic)(void* object);
};

struct RenderManagedObject {
    RenderManagedObjectDispatch* dispatch;
};

struct RenderManagedObjectArray {
    RenderManagedObject** begin;
    RenderManagedObject** end;
    RenderManagedObject** capacity;
    void* allocator;
};

static_assert(sizeof(RenderManagedObjectArray) == 32);

// The primary shader keeps one compiled-object vector per shader stage,
// indexed by RenderShaderStage.
constexpr std::size_t kRenderShaderStageCount = 6;
// Permutation keys are kept per variant program bit; hull and domain share
// one key.
constexpr std::size_t kRenderShaderProgramKeyCount = 5;

void render_compiled_shader_objects_resize(
    RenderManagedObjectArray& objects,
    std::size_t count);
bool render_compiled_shader_objects_load(
    RenderManagedObjectArray (&objects)[kRenderShaderStageCount],
    void* metadata,
    BinStream& stream);

bool render_compiled_shader_objects_bind(
    RenderManagedObjectArray (&objects)[kRenderShaderStageCount],
    RenderContext& context,
    std::int32_t variant,
    const std::uint64_t (&keys)[kRenderShaderProgramKeyCount]);

}  // namespace rb4
