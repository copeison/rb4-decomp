#pragma once

#include <cstddef>

#include "render/platform/orbis/buffers/orbis_compute_buffer.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void orbis_compute_buffer_install_vtable(OrbisComputeBuffer& buffer);
void orbis_defer_compute_buffer_release(void* allocation);
void orbis_compute_buffer_initialize_storage(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_bind_stage(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    RenderShaderStage stage,
    std::uint32_t slot,
    std::uint32_t flags);

}  // namespace rb4
