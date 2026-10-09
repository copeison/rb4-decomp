#include "render/platform/orbis/buffers/orbis_transient_vertex_buffer.h"

#include <cstring>

#include "os/memory/MemMgr.h"
#include "render/platform/orbis/meshes/orbis_gnm_mesh_api.h"
#include "render/platform/orbis/meshes/orbis_mesh_formats.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"

namespace rb4 {

namespace {

constexpr const char* kAllocationName = "TransientBuffer";

}  // namespace

// Reconstructed from eboot.elf at 0x8EC7C0.
void orbis_transient_vertex_buffer_construct(
    OrbisTransientVertexBuffer& buffer) {
    buffer.data = nullptr;
    buffer.vertex_stride = 0;
    buffer.vertex_capacity = 0;
    buffer.vertex_count = 0;
}

// Reconstructed from eboot.elf at 0x8EC7D0.
void orbis_transient_vertex_buffer_destruct(
    OrbisTransientVertexBuffer& buffer) {
    MemFree(buffer.data);
}

// Reconstructed from eboot.elf at 0x8EC7E0.
void orbis_transient_vertex_buffer_initialize(
    OrbisTransientVertexBuffer& buffer,
    RenderMeshFormat format,
    std::size_t vertex_capacity) {
    const auto* descriptor = render_mesh_format_descriptor(format);
    buffer.data = static_cast<std::uint8_t*>(MemAlloc(
        vertex_capacity * descriptor->vertex_stride,
        kAllocationName,
        4));
    buffer.descriptor_mask = 0;
    orbis_build_mesh_vertex_descriptors(
        buffer.descriptors,
        buffer.data,
        buffer.descriptor_mask,
        static_cast<std::uint32_t>(vertex_capacity),
        *descriptor);
    buffer.vertex_stride = descriptor->vertex_stride;
    buffer.vertex_capacity = vertex_capacity;
}

// Reconstructed from eboot.elf at 0x8EC8B0.
void orbis_transient_vertex_buffer_reset(
    OrbisTransientVertexBuffer& buffer) {
    buffer.vertex_count = 0;
}

// Reconstructed from eboot.elf at 0x8EC8C0.
std::size_t orbis_transient_vertex_buffer_append(
    OrbisTransientVertexBuffer& buffer,
    const void* vertices,
    std::size_t vertex_count) {
    std::memcpy(
        buffer.data + buffer.vertex_count * buffer.vertex_stride,
        vertices,
        vertex_count * buffer.vertex_stride);
    const auto first_vertex = buffer.vertex_count;
    buffer.vertex_count += vertex_count;
    return first_vertex;
}

// Reconstructed from eboot.elf at 0x8EC910.
void orbis_transient_vertex_buffer_bind(
    const OrbisTransientVertexBuffer& buffer,
    OrbisRenderCommandContext& context) {
    const auto* defaults = orbis_default_vertex_descriptors();
    for (std::uint32_t stream = 0;
         stream < kMeshVertexStreamCount;
         ++stream) {
        const auto* descriptor =
            (buffer.descriptor_mask & (1U << stream)) != 0
            ? &buffer.descriptors[stream]
            : &defaults[stream];
        orbis_bind_vertex_buffers(context, stream, 1, descriptor);
    }
}

}  // namespace rb4
