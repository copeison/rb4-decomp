#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisConstantBuffer;
struct RenderConstantBufferDescriptor;

OrbisConstantBuffer* orbis_create_constant_buffer(
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count);

}  // namespace rb4
