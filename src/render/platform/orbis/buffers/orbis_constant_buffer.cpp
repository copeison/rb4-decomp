#include "render/platform/orbis/buffers/orbis_constant_buffer.h"

#include "render/platform/orbis/buffers/orbis_constant_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kConstantBufferHeaderSize = 112;
constexpr std::size_t kConstantBufferElementSize = 16;
constexpr const char* kConstantBufferAllocationName = "cbuffer";

}  // namespace

// Reconstructed from eboot.elf at 0x8D8AD0.
OrbisConstantBuffer* orbis_create_constant_buffer(
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count) {
    const auto allocation_size =
        kConstantBufferHeaderSize + kConstantBufferElementSize * element_count;
    auto* storage = render_allocate_named(
        allocation_size, kConstantBufferAllocationName, 0);
    auto* buffer = reinterpret_cast<OrbisConstantBuffer*>(storage);
    orbis_constant_buffer_construct(
        *buffer,
        descriptor,
        flags,
        element_count,
        static_cast<unsigned char*>(storage) + kConstantBufferHeaderSize);
    return buffer;
}

}  // namespace rb4
