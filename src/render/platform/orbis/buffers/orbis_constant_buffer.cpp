#include "render/platform/orbis/buffers/orbis_constant_buffer.h"

#include <algorithm>
#include <cstring>

#include "core/memory/engine_memory.h"
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
    render_constant_buffer_construct(
        buffer, descriptor, flags, element_count, inline_data);
    orbis_constant_buffer_install_vtable(buffer);
    buffer.frame_data = nullptr;
    buffer.gpu_data = nullptr;
    buffer.gpu_size = 0;
    buffer.frame_stamp = 0;
}

// Reconstructed from eboot.elf at 0x8E3830.
void orbis_constant_buffer_destruct(OrbisConstantBuffer& buffer) {
    orbis_constant_buffer_release_backend(buffer);
    render_constant_buffer_destruct(buffer);
}

// Reconstructed from eboot.elf at 0x8E38C0.
void orbis_constant_buffer_delete(OrbisConstantBuffer& buffer) {
    orbis_constant_buffer_destruct(buffer);
    render_constant_buffer_delete(buffer);
}

// Reconstructed from eboot.elf at 0x8E3880.
void orbis_constant_buffer_release_backend(OrbisConstantBuffer& buffer) {
    if (buffer.gpu_data != nullptr) {
        orbis_defer_constant_buffer_release(buffer.gpu_data);
        buffer.gpu_data = nullptr;
    }
    buffer.gpu_size = 0;
}

// Reconstructed from eboot.elf at 0x8E3900.
void orbis_constant_buffer_initialize_backend(OrbisConstantBuffer& buffer) {
    orbis_constant_buffer_release_backend(buffer);
    buffer.gpu_size = std::max(
        kConstantBufferElementSize,
        kConstantBufferElementSize * buffer.element_count);
    buffer.gpu_data = orbis_allocate_constant_buffer_storage(
        buffer.gpu_size,
        kConstantBufferGpuAllocationName,
        kConstantBufferGpuAlignment);
    std::memcpy(buffer.gpu_data, buffer.data, buffer.gpu_size);
    buffer.frame_data = nullptr;
}

// Reconstructed from eboot.elf at 0x8E3980.
void orbis_constant_buffer_update_range(
    OrbisConstantBuffer& buffer,
    OrbisRenderContext&,
    std::size_t first_element,
    std::size_t end_element) {
    auto* destination = static_cast<std::uint8_t*>(buffer.gpu_data) +
        kConstantBufferElementSize * first_element;
    const auto byte_count =
        kConstantBufferElementSize * (end_element - first_element);
    std::memcpy(destination, buffer.data, byte_count);
    buffer.frame_data = nullptr;
}

// Reconstructed from eboot.elf at 0x8E39C0.
void orbis_constant_buffer_bind(
    OrbisConstantBuffer& buffer,
    OrbisRenderContext& context) {
    orbis_constant_buffer_prepare_frame_data(buffer, context);
    orbis_constant_buffer_bind_stage_mask(buffer, context);
}

}  // namespace rb4
