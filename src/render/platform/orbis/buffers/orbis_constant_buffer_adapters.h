#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/buffers/orbis_constant_buffer.h"

namespace rb4 {

void orbis_constant_buffer_install_vtable(OrbisConstantBuffer& buffer);
void orbis_defer_constant_buffer_release(void* allocation);
void* orbis_allocate_constant_buffer_storage(
    std::size_t size,
    const char* allocation_name,
    std::size_t alignment);
void orbis_constant_buffer_prepare_frame_data(
    OrbisConstantBuffer& buffer,
    OrbisRenderContext& context);
void orbis_constant_buffer_bind_stage_mask(
    const OrbisConstantBuffer& buffer,
    OrbisRenderContext& context);

}  // namespace rb4
