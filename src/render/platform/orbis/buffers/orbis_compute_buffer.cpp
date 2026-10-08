#include "render/platform/orbis/buffers/orbis_compute_buffer.h"

#include <cstddef>

#include "render/platform/orbis/buffers/orbis_compute_buffer_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisComputeBufferSize = 136;

}  // namespace

// Reconstructed from eboot.elf at 0x8D8BC0.
OrbisComputeBuffer* orbis_create_compute_buffer(
    const RenderComputeBufferDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisComputeBufferSize);
    auto* buffer = reinterpret_cast<OrbisComputeBuffer*>(storage);
    orbis_compute_buffer_construct(*buffer, descriptor);
    return buffer;
}

// Reconstructed from eboot.elf at 0x8E3250.
void orbis_compute_buffer_construct(
    OrbisComputeBuffer& buffer,
    const RenderComputeBufferDescriptor& descriptor) {
    compute_buffer_construct(buffer, descriptor);
    orbis_compute_buffer_clear_backend_state(buffer);
}

// Reconstructed from eboot.elf at 0x8E3290.
void orbis_compute_buffer_destruct(OrbisComputeBuffer& buffer) {
    orbis_compute_buffer_release_backend(buffer);
    compute_buffer_destruct(buffer);
}

// Reconstructed from eboot.elf at 0x8E32F0.
void orbis_compute_buffer_delete(OrbisComputeBuffer& buffer) {
    orbis_compute_buffer_destruct(buffer);
    render_delete_compute_buffer(buffer);
}

// Reconstructed from eboot.elf at 0x8E3350.
bool orbis_compute_buffer_initialize_backend(OrbisComputeBuffer& buffer) {
    orbis_compute_buffer_release_backend(buffer);
    orbis_compute_buffer_initialize_storage(buffer);
    return true;
}

// Reconstructed from eboot.elf at 0x8E34D0.
void orbis_compute_buffer_release_backend(OrbisComputeBuffer& buffer) {
    orbis_compute_buffer_release_allocations(buffer);
}

// Reconstructed from eboot.elf at 0x8E3510.
void orbis_compute_buffer_update_gpu_data(OrbisComputeBuffer& buffer) {
    orbis_compute_buffer_flip_active_storage(buffer);
    orbis_compute_buffer_upload_active_data(buffer);
}

// Reconstructed from eboot.elf at 0x8E3580.
void orbis_compute_buffer_bind_vertex(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot) {
    orbis_compute_buffer_bind_stage(
        buffer, context, RenderShaderStage::kVertex, slot, 0);
}

// Reconstructed from eboot.elf at 0x8E3600.
void orbis_compute_buffer_bind_hull(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot) {
    orbis_compute_buffer_bind_stage(
        buffer, context, RenderShaderStage::kHull, slot, 0);
}

// Reconstructed from eboot.elf at 0x8E3630.
void orbis_compute_buffer_bind_domain(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot) {
    orbis_compute_buffer_bind_stage(
        buffer, context, RenderShaderStage::kDomain, slot, 0);
}

// Reconstructed from eboot.elf at 0x8E3660.
void orbis_compute_buffer_bind_geometry(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot) {
    orbis_compute_buffer_bind_stage(
        buffer, context, RenderShaderStage::kGeometry, slot, 0);
}

// Reconstructed from eboot.elf at 0x8E3690.
void orbis_compute_buffer_bind_pixel(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags) {
    orbis_compute_buffer_bind_stage(
        buffer, context, RenderShaderStage::kPixel, slot, flags);
}

// Reconstructed from eboot.elf at 0x8E36D0.
void orbis_compute_buffer_bind_compute(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags) {
    orbis_compute_buffer_bind_stage(
        buffer, context, RenderShaderStage::kCompute, slot, flags);
}

}  // namespace rb4
