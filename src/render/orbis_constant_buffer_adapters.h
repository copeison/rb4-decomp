#pragma once

#include <cstddef>
#include <cstdint>

#include "orbis_constant_buffer.h"

namespace rb4 {

void* render_allocate_named(
    std::size_t size,
    const char* name,
    std::uint32_t alignment);
void orbis_constant_buffer_construct(
    OrbisConstantBuffer& buffer,
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count,
    void* inline_data);

}  // namespace rb4
