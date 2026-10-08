#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisFence;
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

}  // namespace rb4
