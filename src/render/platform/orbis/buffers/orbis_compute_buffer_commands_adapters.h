#pragma once

#include <cstddef>
#include <cstdint>

#include "renderps4/buffers/PS4ComputeBuffer.h"

class PS4Context;

namespace rb4 {

void orbis_render_context_bind_compute_rw_buffer(
    PS4Context& context,
    std::uint32_t slot,
    const PS4ComputeBuffer::GnmBuffer* descriptor);
void orbis_render_context_copy_gds_to_memory(
    PS4Context& context,
    std::uint32_t gds_offset,
    void* destination,
    std::size_t size,
    bool blocking);

}  // namespace rb4
