#include "render/core/buffers/render_compute_buffer.h"

#include "render/core/buffers/render_compute_buffer_adapters.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x636C70.
RenderComputeBuffer* render_create_compute_buffer(
    const RenderComputeBufferDescriptor& descriptor) {
    auto& factory = *render_system_factory(*render_system_instance());
    auto* buffer = render_factory_create_compute_buffer(factory, descriptor);
    const auto staging_size =
        buffer->element_count * buffer->element_stride;
    buffer->staging_data =
        render_allocate_compute_buffer_staging(staging_size);
    render_compute_buffer_initialize_backend(*buffer);
    return buffer;
}

// Reconstructed from eboot.elf at 0x636CC0.
void render_compute_buffer_construct(
    RenderComputeBuffer& buffer,
    const RenderComputeBufferDescriptor& descriptor) {
    render_compute_buffer_set_base_dispatch(buffer);
    buffer.frame_stamp = -1;
    buffer.element_count = descriptor.element_count;
    buffer.element_stride = descriptor.element_stride;
    buffer.initial_data = descriptor.initial_data;
    buffer.external_gpu_data = descriptor.external_gpu_data;
    buffer.reserved = descriptor.reserved;
    buffer.flags = descriptor.flags;
    buffer.name = descriptor.name;
    buffer.staging_data = nullptr;
}

// Reconstructed from eboot.elf at 0x636D10.
void render_compute_buffer_destruct(RenderComputeBuffer& buffer) {
    if (buffer.staging_data != nullptr) {
        render_free_compute_buffer_staging(buffer.staging_data);
    }
}

// Reconstructed from eboot.elf at 0x636D50.
void render_compute_buffer_delete(RenderComputeBuffer& buffer) {
    render_compute_buffer_destruct(buffer);
    render_delete_compute_buffer_storage(buffer);
}

// Reconstructed from eboot.elf at 0x636DB0.
std::uint32_t render_compute_buffer_type(const RenderComputeBuffer&) {
    return UINT32_MAX;
}

}  // namespace rb4
