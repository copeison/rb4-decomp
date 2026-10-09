#include "render/core/buffers/render_constant_buffer.h"

#include "core/memory/engine_memory.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kDeferInitialUpload = 1U << 0;

RenderConstantBufferDispatch kBaseConstantBufferDispatch{
    render_constant_buffer_destruct,
    render_constant_buffer_delete,
    nullptr,
    nullptr,
    nullptr,
};

void set_base_dispatch(RenderConstantBuffer& buffer) {
    buffer.dispatch = &kBaseConstantBufferDispatch;
}

}  // namespace

// Reconstructed from eboot.elf at 0x639F30.
RenderConstantBuffer* render_create_constant_buffer(
    const RenderConstantBufferDescriptor& descriptor,
    std::uint32_t flags,
    std::size_t element_count) {
    if (element_count == kUseConstantBufferDescriptorCount) {
        element_count = descriptor.element_count;
    }

    auto& factory = *render_system_factory(*render_system_instance());
    auto* buffer = render_factory_create_constant_buffer(
        factory, descriptor, flags, element_count);
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
    set_base_dispatch(buffer);
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

void render_delete_constant_buffer_storage(RenderConstantBuffer& buffer) {
    render_release(&buffer);
}

void render_constant_buffer_initialize_backend(RenderConstantBuffer& buffer) {
    buffer.dispatch->initialize_backend(buffer);
}

void render_constant_buffer_release_dynamic(RenderConstantBuffer& buffer) {
    buffer.dispatch->destruct(buffer);
    render_release(&buffer);
}


void render_constant_buffer_update_range(
    RenderConstantBuffer& buffer,
    RenderContext& context,
    std::size_t first_element,
    std::size_t end_element) {
    buffer.dispatch->update_range(buffer, context, first_element, end_element);
}

void render_constant_buffer_bind(
    RenderConstantBuffer& buffer,
    RenderContext& context) {
    buffer.dispatch->bind(buffer, context);
}

}  // namespace rb4
