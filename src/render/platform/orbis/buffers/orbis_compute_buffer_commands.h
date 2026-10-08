#pragma once

namespace rb4 {

struct OrbisComputeBuffer;
struct OrbisRenderContext;

void orbis_render_context_copy_compute_buffer_count(
    OrbisRenderContext& context,
    const OrbisComputeBuffer& source,
    OrbisComputeBuffer& destination);

}  // namespace rb4
