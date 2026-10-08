#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/render_constant_buffer.h"

namespace rb4 {

struct OrbisRenderContext;

struct OrbisConstantBuffer : RenderConstantBuffer {
    std::uint8_t backend_reserved[16];
    void* frame_data;
    void* gpu_data;
    std::size_t gpu_size;
    std::uint64_t frame_stamp;
};

static_assert(sizeof(OrbisConstantBuffer) == 112);

OrbisConstantBuffer* orbis_create_constant_buffer(
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count);
void orbis_constant_buffer_construct(
    OrbisConstantBuffer& buffer,
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count,
    void* inline_data);
void orbis_constant_buffer_destruct(OrbisConstantBuffer& buffer);
void orbis_constant_buffer_delete(OrbisConstantBuffer& buffer);
void orbis_constant_buffer_release_backend(OrbisConstantBuffer& buffer);
void orbis_constant_buffer_initialize_backend(OrbisConstantBuffer& buffer);
void orbis_constant_buffer_update_range(
    OrbisConstantBuffer& buffer,
    OrbisRenderContext& context,
    std::size_t first_element,
    std::size_t end_element);
void orbis_constant_buffer_bind(
    OrbisConstantBuffer& buffer,
    OrbisRenderContext& context);

}  // namespace rb4
