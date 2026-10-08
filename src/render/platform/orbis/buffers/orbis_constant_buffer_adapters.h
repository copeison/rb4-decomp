#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/buffers/orbis_constant_buffer.h"

namespace rb4 {

void* render_allocate_named(
    std::size_t size,
    const char* name,
    std::uint32_t alignment);
void constant_buffer_construct(
    OrbisConstantBuffer& buffer,
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count,
    void* inline_data);
void orbis_constant_buffer_clear_backend_state(OrbisConstantBuffer& buffer);
void orbis_constant_buffer_release_allocation(OrbisConstantBuffer& buffer);
void render_delete_constant_buffer(OrbisConstantBuffer& buffer);
void orbis_constant_buffer_initialize_storage(
    OrbisConstantBuffer& buffer,
    const char* allocation_name,
    std::size_t alignment);
void orbis_constant_buffer_copy_range(
    OrbisConstantBuffer& buffer,
    std::size_t first_element,
    std::size_t end_element);
void orbis_constant_buffer_prepare_frame_data(
    OrbisConstantBuffer& buffer,
    OrbisRenderContext& context);
void orbis_constant_buffer_bind_stage_mask(
    const OrbisConstantBuffer& buffer,
    OrbisRenderContext& context);

}  // namespace rb4
