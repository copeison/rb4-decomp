#pragma once

class PS4ComputeBuffer;

namespace rb4 {

struct OrbisRenderContext;

void orbis_render_context_copy_compute_buffer_count(
    OrbisRenderContext& context,
    const PS4ComputeBuffer& source,
    PS4ComputeBuffer& destination);

}  // namespace rb4
