#pragma once

#include <cstddef>
#include <cstdint>

#include "render/meshes/RndMesh.h"
#include "render/platform/orbis/meshes/orbis_vertex_descriptors.h"

namespace rb4 {

struct OrbisRenderCommandContext;

struct OrbisTransientVertexBuffer {
    OrbisBufferDescriptor descriptors[kMeshVertexStreamCount];
    std::uint32_t descriptor_mask;
    std::uint32_t reserved_132;
    std::uint8_t* data;
    std::size_t vertex_stride;
    std::size_t vertex_capacity;
    std::size_t vertex_count;
};

static_assert(sizeof(OrbisTransientVertexBuffer) == 168);
static_assert(offsetof(OrbisTransientVertexBuffer, descriptor_mask) == 128);
static_assert(offsetof(OrbisTransientVertexBuffer, data) == 136);
static_assert(offsetof(OrbisTransientVertexBuffer, vertex_stride) == 144);
static_assert(offsetof(OrbisTransientVertexBuffer, vertex_capacity) == 152);
static_assert(offsetof(OrbisTransientVertexBuffer, vertex_count) == 160);

void orbis_transient_vertex_buffer_construct(
    OrbisTransientVertexBuffer& buffer);
void orbis_transient_vertex_buffer_destruct(
    OrbisTransientVertexBuffer& buffer);
void orbis_transient_vertex_buffer_initialize(
    OrbisTransientVertexBuffer& buffer,
    RndVertexType format,
    std::size_t vertex_capacity);
void orbis_transient_vertex_buffer_reset(
    OrbisTransientVertexBuffer& buffer);
std::size_t orbis_transient_vertex_buffer_append(
    OrbisTransientVertexBuffer& buffer,
    const void* vertices,
    std::size_t vertex_count);
void orbis_transient_vertex_buffer_bind(
    const OrbisTransientVertexBuffer& buffer,
    OrbisRenderCommandContext& context);

}  // namespace rb4
