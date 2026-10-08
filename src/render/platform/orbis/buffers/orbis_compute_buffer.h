#pragma once

#include <cstdint>

#include "render/core/render_compute_buffer.h"
#include "render/core/render_shader.h"

namespace rb4 {

struct OrbisRenderContext;

struct OrbisGnmBufferDescriptor {
    std::uint8_t data[16];
};

struct OrbisComputeBuffer : RenderComputeBuffer {
    OrbisGnmBufferDescriptor descriptors[2];
    void* allocations[2];
    std::size_t active_bank;
};

static_assert(sizeof(OrbisComputeBuffer) == 136);

OrbisComputeBuffer* orbis_create_compute_buffer(
    const RenderComputeBufferDescriptor& descriptor);
void orbis_compute_buffer_construct(
    OrbisComputeBuffer& buffer,
    const RenderComputeBufferDescriptor& descriptor);
void orbis_compute_buffer_destruct(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_delete(OrbisComputeBuffer& buffer);
bool orbis_compute_buffer_initialize_backend(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_release_backend(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_update_gpu_data(OrbisComputeBuffer& buffer);
void orbis_compute_buffer_bind_vertex(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot);
void orbis_compute_buffer_bind_hull(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot);
void orbis_compute_buffer_bind_domain(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot);
void orbis_compute_buffer_bind_geometry(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot);
void orbis_compute_buffer_bind_pixel(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags);
void orbis_compute_buffer_bind_compute(
    const OrbisComputeBuffer& buffer,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags);

}  // namespace rb4
