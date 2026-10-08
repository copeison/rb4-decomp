#include "render/core/buffers/render_constant_buffer.h"

#include "render/core/buffers/render_constant_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kDeferInitialUpload = 1U << 0;

}  // namespace

// Reconstructed from eboot.elf at 0x639F30.
RenderConstantBuffer* render_create_constant_buffer(
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count) {
    if (element_count == kUseConstantBufferDescriptorCount) {
        element_count = descriptor.element_count;
    }

    auto* buffer = render_system_create_constant_buffer(
        descriptor, flags, element_count);
    if ((flags & kDeferInitialUpload) == 0 && buffer->upload_pending) {
        render_constant_buffer_initialize_backend(*buffer);
        buffer->upload_pending = false;
    }
    return buffer;
}

// Reconstructed from eboot.elf at 0x639FF0.
void render_constant_buffer_construct(
    RenderConstantBuffer& buffer,
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count,
    void* data) {
    render_constant_buffer_set_base_dispatch(buffer);
    buffer.owner = descriptor.owner;
    buffer.flags = flags;
    buffer.slot = descriptor.slot;
    buffer.stage_mask = descriptor.stage_mask;
    buffer.descriptor_element_count = descriptor.element_count;
    buffer.element_count = element_count;
    buffer.data = data;
    buffer.upload_pending = true;
}

// Reconstructed from eboot.elf at 0x63A030.
void render_constant_buffer_destruct(RenderConstantBuffer&) {
}

// Reconstructed from eboot.elf at 0x63A040.
void render_constant_buffer_delete(RenderConstantBuffer& buffer) {
    render_delete_constant_buffer_storage(buffer);
}

}  // namespace rb4
