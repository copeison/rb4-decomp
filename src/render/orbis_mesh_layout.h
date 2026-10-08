#pragma once

#include <cstddef>
#include <cstdint>

#include "orbis_mesh.h"
#include "orbis_vertex_descriptors.h"

namespace rb4 {

template <typename Vertex>
struct OrbisMeshLayout {
    std::uint8_t base_to_vertex_count[56];
    std::size_t vertex_count;
    std::uint8_t base_tail[64];
    Vertex* vertices_begin;
    Vertex* vertices_end;
    Vertex* vertices_capacity_end;
    std::uint8_t descriptor_padding[8];
    OrbisBufferDescriptor vertex_descriptors[2][kMeshVertexStreamCount];
    Vertex* vertex_buffers[2];
    std::size_t active_vertex_buffer;
    std::size_t vertex_buffer_capacity;
    std::uint32_t descriptor_mask;
    std::uint32_t index_format;
    void* index_buffer;
    std::size_t index_buffer_capacity;
};

template <typename Vertex>
OrbisMeshLayout<Vertex>& mesh_layout(OrbisMesh& mesh) {
    return reinterpret_cast<OrbisMeshLayout<Vertex>&>(mesh);
}

template <typename Vertex>
const OrbisMeshLayout<Vertex>& mesh_layout(const OrbisMesh& mesh) {
    return reinterpret_cast<const OrbisMeshLayout<Vertex>&>(mesh);
}

template <typename Vertex>
std::size_t mesh_vertex_count(const OrbisMesh& mesh) {
    const auto& layout = mesh_layout<Vertex>(mesh);
    return static_cast<std::size_t>(
        layout.vertices_end - layout.vertices_begin);
}

inline bool has_mesh_update_flag(
    MeshUpdateFlags flags,
    MeshUpdateFlags flag) {
    return (static_cast<std::uint32_t>(flags) &
            static_cast<std::uint32_t>(flag)) != 0;
}

}  // namespace rb4
