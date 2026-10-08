#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/render_constant_buffer.h"

namespace rb4 {

RenderConstantBuffer* render_system_create_constant_buffer(
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count);
void render_constant_buffer_set_base_dispatch(RenderConstantBuffer& buffer);
void render_constant_buffer_initialize_backend(RenderConstantBuffer& buffer);
void render_delete_constant_buffer_storage(RenderConstantBuffer& buffer);

}  // namespace rb4
