#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisFence;
struct OrbisRenderContext;
struct OrbisRenderSystem;

void* render_allocate(std::size_t size);
std::uint32_t* orbis_allocate_fence_value(
    std::size_t size,
    const char* name,
    std::uint32_t alignment);
void orbis_fence_set_value_storage(
    OrbisFence& fence,
    std::uint32_t* value);
std::uint32_t* orbis_fence_value_storage(const OrbisFence& fence);
OrbisRenderSystem* current_orbis_render_system();
void render_release(void* allocation);
void render_delete_fence(OrbisFence& fence);
std::uint32_t orbis_fence_sequence(const OrbisFence& fence);
void orbis_fence_set_sequence(
    OrbisFence& fence,
    std::uint32_t sequence);
bool orbis_render_context_recording_graphics(
    const OrbisRenderContext& context);
bool orbis_render_context_recording_compute(
    const OrbisRenderContext& context);
void orbis_render_context_emit_graphics_fence_signal(
    OrbisRenderContext& context,
    std::uint32_t* address,
    std::uint32_t value);
void orbis_render_context_emit_compute_fence_signal(
    OrbisRenderContext& context,
    std::uint32_t* address,
    std::uint32_t value);
void orbis_render_context_emit_graphics_fence_wait(
    OrbisRenderContext& context,
    const std::uint32_t* address,
    std::uint32_t value);
void orbis_render_context_emit_compute_fence_wait(
    OrbisRenderContext& context,
    const std::uint32_t* address,
    std::uint32_t value);

}  // namespace rb4
