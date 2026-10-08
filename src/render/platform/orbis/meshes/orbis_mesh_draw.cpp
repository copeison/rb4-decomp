#include "render/platform/orbis/meshes/orbis_mesh_draw.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "render/platform/orbis/meshes/orbis_mesh_adapters.h"
#include "render/platform/orbis/meshes/orbis_mesh_layout.h"

namespace rb4 {

namespace {

constexpr std::size_t kIndicesPerTriangle = 3;

using MeshDrawLayout = OrbisMeshLayout<std::uint8_t>;

static_assert(offsetof(MeshDrawLayout, triangle_count) == 64,
              "unexpected mesh triangle-count offset");
static_assert(offsetof(MeshDrawLayout, last_draw_frame) == 112,
              "unexpected mesh frame-stamp offset");
static_assert(offsetof(MeshDrawLayout, vertex_descriptors) == 160,
              "unexpected mesh descriptor offset");
static_assert(offsetof(MeshDrawLayout, active_vertex_buffer) == 432,
              "unexpected active vertex-buffer offset");
static_assert(offsetof(MeshDrawLayout, descriptor_mask) == 448,
              "unexpected mesh descriptor-mask offset");
static_assert(offsetof(MeshDrawLayout, index_buffer) == 456,
              "unexpected mesh index-buffer offset");

void bind_mesh_vertex_descriptors(
    OrbisRenderCommandContext& context,
    const MeshDrawLayout& mesh) {
    const auto* defaults = orbis_default_vertex_descriptors();
    const auto* mesh_descriptors =
        mesh.vertex_descriptors[mesh.active_vertex_buffer];

    for (std::uint32_t stream = 0;
         stream < kMeshVertexStreamCount;
         ++stream) {
        const bool mesh_provides_stream =
            (mesh.descriptor_mask & (1U << stream)) != 0;
        const auto* descriptor = mesh_provides_stream
            ? &mesh_descriptors[stream]
            : &defaults[stream];
        orbis_bind_vertex_buffers(context, stream, 1, descriptor);
    }
}

void bind_instance_vertex_descriptors(
    OrbisRenderCommandContext& context,
    const MeshInstanceBatch& instances) {
    const auto byte_count = sizeof(OrbisMeshInstanceData) * instances.count;
    auto* uploaded = static_cast<OrbisMeshInstanceData*>(
        orbis_allocate_embedded_data(context, byte_count, 4));

    OrbisBufferDescriptor descriptors[kInstanceVertexStreamCount] = {};
    orbis_build_instance_vertex_descriptors(
        descriptors, uploaded, static_cast<std::uint32_t>(instances.count));
    std::memcpy(uploaded, instances.instances, byte_count);
    orbis_bind_vertex_buffers(
        context,
        static_cast<std::uint32_t>(kMeshVertexStreamCount),
        static_cast<std::uint32_t>(kInstanceVertexStreamCount),
        descriptors);
    gnm_draw_command_buffer_set_num_instances(
        context, static_cast<std::uint32_t>(instances.count));
}

void draw_indexed(
    OrbisRenderCommandContext& context,
    const MeshDrawLayout& mesh,
    const MeshDrawRange& range) {
    const auto triangle_count = range.triangle_count ==
            MeshDrawRange::kAllTriangles
        ? mesh.triangle_count
        : range.triangle_count;
    const auto index_count = static_cast<std::uint32_t>(
        kIndicesPerTriangle * triangle_count);
    const auto index_size = static_cast<OrbisIndexSize>(mesh.index_format);
    const auto index_byte_size = index_size == OrbisIndexSize::k32Bit ? 4U : 2U;
    const auto byte_offset =
        kIndicesPerTriangle * index_byte_size * range.first_triangle;
    const auto* index_data =
        static_cast<const std::uint8_t*>(mesh.index_buffer) + byte_offset;

    gnm_draw_command_buffer_set_index_size(
        context, index_size, OrbisCachePolicy::kBypass);
    gnmx_prepare_draw(context);
    gnm_draw_command_buffer_draw_index(context, index_count, index_data);
}

void draw_nonindexed(
    OrbisRenderCommandContext& context,
    const MeshDrawLayout& mesh) {
    gnmx_prepare_draw(context);
    gnm_draw_command_buffer_draw_index_auto(
        context, static_cast<std::uint32_t>(mesh.vertex_count));
}

}  // namespace

// Reconstructed from the equivalent format-specialized implementations at
// 0x8D8E00, 0x8D9FB0, 0x8DB310, 0x8DC4F0, 0x8DD910, 0x8DED60, and 0x8E0200.
void orbis_mesh_draw(
    OrbisMesh& mesh,
    OrbisRenderCommandContext& context,
    const MeshInstanceBatch& instances,
    const MeshDrawRange& range) {
    auto& layout = mesh_layout<std::uint8_t>(mesh);
    bind_mesh_vertex_descriptors(context, layout);
    bind_instance_vertex_descriptors(context, instances);
    orbis_set_primitive_type(context, MeshPrimitiveType::kTriangles);

    if (layout.index_buffer != nullptr) {
        draw_indexed(context, layout, range);
    } else {
        draw_nonindexed(context, layout);
    }

    gnmx_finish_draw(context);
    gnm_draw_command_buffer_set_num_instances(context, 1);
    layout.last_draw_frame = render_frame_counter();
}

}  // namespace rb4
