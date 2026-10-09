#include "render/core/buffers/render_compute_buffer.h"

#include "os/memory/MemMgr.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

namespace {

struct RenderComputeBufferDispatch {
    void (*destruct)(RenderComputeBuffer& buffer);
    void (*release_dynamic)(RenderComputeBuffer& buffer);
    std::uint32_t (*type)(const RenderComputeBuffer& buffer);
    // Slots 3-8 bind the buffer for the vertex, hull, domain, geometry,
    // pixel, and compute stages.
    void (*bind_stage[6])(
        const RenderComputeBuffer& buffer,
        RenderContext& context,
        std::uint32_t slot,
        std::uint32_t flags);
    void (*reserved_72)();
    void (*initialize_backend)(RenderComputeBuffer& buffer);
};

static_assert(offsetof(RenderComputeBufferDispatch, release_dynamic) == 8);
static_assert(offsetof(RenderComputeBufferDispatch, initialize_backend) == 80);

RenderComputeBufferDispatch kBaseComputeBufferDispatch{
    render_compute_buffer_destruct,
    render_compute_buffer_delete,
    render_compute_buffer_type,
    {},
    nullptr,
    nullptr,
};

const RenderComputeBufferDispatch& dispatch(
    const RenderComputeBuffer& buffer) {
    return *static_cast<const RenderComputeBufferDispatch*>(
        buffer.implementation);
}

void set_base_dispatch(RenderComputeBuffer& buffer) {
    buffer.implementation = &kBaseComputeBufferDispatch;
}

}  // namespace

// Reconstructed from eboot.elf at 0x636C70.
RenderComputeBuffer* render_create_compute_buffer(
    const RenderComputeBufferDescriptor& descriptor) {
    auto& factory = *render_system_factory(*render_system_instance());
    auto* buffer = render_factory_create_compute_buffer(factory, descriptor);
    const auto staging_size =
        buffer->element_count * buffer->element_stride;
    buffer->staging_data = operator new(staging_size);
    render_compute_buffer_initialize_backend(*buffer);
    return buffer;
}

// Reconstructed from eboot.elf at 0x636CC0.
void render_compute_buffer_construct(
    RenderComputeBuffer& buffer,
    const RenderComputeBufferDescriptor& descriptor) {
    set_base_dispatch(buffer);
    buffer.frame_stamp = -1;
    buffer.element_stride = descriptor.element_stride;
    buffer.element_count = descriptor.element_count;
    buffer.initial_data = descriptor.initial_data;
    buffer.external_gpu_data = descriptor.external_gpu_data;
    buffer.reserved = descriptor.reserved;
    buffer.flags = descriptor.flags;
    buffer.name = descriptor.name;
    buffer.staging_data = nullptr;
}

// Reconstructed from eboot.elf at 0x636D10.
void render_compute_buffer_destruct(RenderComputeBuffer& buffer) {
    set_base_dispatch(buffer);
    if (buffer.staging_data != nullptr) {
        MemFree(buffer.staging_data);
    }
}

// Reconstructed from eboot.elf at 0x636D50.
void render_compute_buffer_delete(RenderComputeBuffer& buffer) {
    render_compute_buffer_destruct(buffer);
    render_delete_compute_buffer_storage(buffer);
}

void render_delete_compute_buffer_storage(RenderComputeBuffer& buffer) {
    MemFree(&buffer);
}

void render_compute_buffer_release_dynamic(RenderComputeBuffer& buffer) {
    dispatch(buffer).release_dynamic(buffer);
}

void render_compute_buffer_initialize_backend(RenderComputeBuffer& buffer) {
    dispatch(buffer).initialize_backend(buffer);
}

// Reconstructed from eboot.elf at 0x636DB0.
std::uint32_t render_compute_buffer_type(const RenderComputeBuffer&) {
    return UINT32_MAX;
}

void render_compute_buffer_bind(
    const RenderComputeBuffer& buffer,
    RenderContext& context,
    RenderShaderStage stage,
    std::uint32_t slot,
    std::uint32_t flags) {
    dispatch(buffer).bind_stage[static_cast<std::size_t>(stage)](
        buffer, context, slot, flags);
}

}  // namespace rb4
