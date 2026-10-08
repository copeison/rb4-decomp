#pragma once

#include <cstddef>

#include "orbis_compute_buffer.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void compute_buffer_construct(
    OrbisComputeBuffer& buffer,
    const RenderComputeBufferDescriptor& descriptor);
void orbis_compute_buffer_clear_backend_state(OrbisComputeBuffer& buffer);

}  // namespace rb4
