#include "render/platform/orbis/buffers/orbis_constant_buffer.h"

#include "render/platform/orbis/buffers/orbis_constant_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kConstantBufferHeaderSize = 112;
constexpr std::size_t kConstantBufferElementSize = 16;
constexpr const char* kConstantBufferAllocationName = "cbuffer";
constexpr const char* kConstantBufferGpuAllocationName = "CBuffer";
constexpr std::size_t kConstantBufferGpuAlignment = 4;

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

// Reconstructed from eboot.elf at 0x8E3800.
void orbis_constant_buffer_construct(
    OrbisConstantBuffer& buffer,
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count,
    void* inline_data) {
    constant_buffer_construct(
        buffer, descriptor, flags, element_count, inline_data);
    orbis_constant_buffer_clear_backend_state(buffer);
}

// Reconstructed from eboot.elf at 0x8E3830.
void orbis_constant_buffer_destruct(OrbisConstantBuffer& buffer) {
    orbis_constant_buffer_release_backend(buffer);
}

// Reconstructed from eboot.elf at 0x8E38C0.
void orbis_constant_buffer_delete(OrbisConstantBuffer& buffer) {
    orbis_constant_buffer_destruct(buffer);
    render_delete_constant_buffer(buffer);
}

// Reconstructed from eboot.elf at 0x8E3880.
void orbis_constant_buffer_release_backend(OrbisConstantBuffer& buffer) {
    orbis_constant_buffer_release_allocation(buffer);
}

// Reconstructed from eboot.elf at 0x8E3900.
void orbis_constant_buffer_initialize_backend(OrbisConstantBuffer& buffer) {
    orbis_constant_buffer_release_backend(buffer);
    orbis_constant_buffer_initialize_storage(
        buffer,
        kConstantBufferGpuAllocationName,
        kConstantBufferGpuAlignment);
}

// Reconstructed from eboot.elf at 0x8E3980.
void orbis_constant_buffer_update_range(
    OrbisConstantBuffer& buffer,
    OrbisRenderContext&,
    std::size_t first_element,
    std::size_t end_element) {
    orbis_constant_buffer_copy_range(buffer, first_element, end_element);
}

// Reconstructed from eboot.elf at 0x8E39C0.
void orbis_constant_buffer_bind(
    OrbisConstantBuffer& buffer,
    OrbisRenderContext& context) {
    orbis_constant_buffer_prepare_frame_data(buffer, context);
    orbis_constant_buffer_bind_stage_mask(buffer, context);
}

}  // namespace rb4
