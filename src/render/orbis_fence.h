#pragma once

#include <cstdint>

namespace rb4 {

struct OrbisFence;
struct OrbisRenderContext;

OrbisFence* orbis_create_fence();
void orbis_fence_construct(OrbisFence& fence);
void orbis_fence_destruct(OrbisFence& fence);
void orbis_fence_base_destruct(OrbisFence& fence);
void orbis_fence_delete(OrbisFence& fence);
std::uint32_t orbis_fence_next_value(OrbisFence& fence);
void orbis_render_context_signal_fence(
    OrbisRenderContext& context,
    OrbisFence& fence);
void orbis_render_context_wait_fence(
    OrbisRenderContext& context,
    const OrbisFence& fence);

}  // namespace rb4
