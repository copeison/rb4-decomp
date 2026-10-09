#pragma once

class PS4Fence;

namespace rb4 {

struct OrbisRenderContext;

void orbis_render_context_signal_fence(
    OrbisRenderContext& context,
    PS4Fence& fence);
void orbis_render_context_wait_fence(
    OrbisRenderContext& context,
    const PS4Fence& fence);

}  // namespace rb4
