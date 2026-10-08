#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisGnmBufferDescriptor;
struct OrbisRenderContext;

void orbis_render_context_bind_compute_rw_buffer(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const OrbisGnmBufferDescriptor* descriptor);
void orbis_render_context_copy_gds_to_memory(
    OrbisRenderContext& context,
    std::uint32_t gds_offset,
    void* destination,
    std::size_t size,
    bool blocking);

}  // namespace rb4
