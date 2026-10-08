#pragma once

#include <cstddef>

#include "render/platform/orbis/buffers/orbis_compute_buffer.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void compute_buffer_construct(
    OrbisComputeBuffer& buffer,
    const RenderComputeBufferDescriptor& descriptor);
void orbis_compute_buffer_clear_backend_state(OrbisComputeBuffer& buffer);
void compute_buffer_destruct(OrbisComputeBuffer& buffer);
void render_delete_compute_buffer(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_initialize_storage(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_release_allocations(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_flip_active_storage(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_upload_active_data(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_bind_stage(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    RenderShaderStage stage,
    std::uint32_t slot,
    std::uint32_t flags);

}  // namespace rb4
