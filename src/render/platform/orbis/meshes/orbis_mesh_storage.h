#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "core/memory/engine_memory.h"
#include "render/platform/orbis/meshes/orbis_mesh_draw.h"
#include "render/platform/orbis/meshes/orbis_mesh_formats.h"
#include "render/platform/orbis/meshes/orbis_mesh_layout.h"
#include "render/platform/orbis/synchronization/orbis_gpu_sync.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"

namespace rb4 {

template <typename Vertex, typename DefaultVertex>
void orbis_mesh_grow_vertex_storage(
    OrbisMesh& mesh,
    std::size_t additional_count,
    DefaultVertex default_vertex) {
    auto& layout = mesh_layout<Vertex>(mesh);
    const auto current_count = mesh_vertex_count<Vertex>(mesh);
    const auto capacity = layout.vertices_begin == nullptr
        ? 0
        : static_cast<std::size_t>(
              layout.vertices_capacity_end - layout.vertices_begin);
    const auto required_count = current_count + additional_count;

    if (required_count > capacity) {
        const auto new_capacity = std::max(
            required_count,
            std::max<std::size_t>(1, current_count * 2));
        auto* replacement = static_cast<Vertex*>(
            engine_allocate_sized(new_capacity * sizeof(Vertex)));
        if (current_count != 0) {
            std::memcpy(
                replacement,
                layout.vertices_begin,
                current_count * sizeof(Vertex));
        }
        if (layout.vertices_begin != nullptr) {
            engine_deallocate_sized(
                layout.vertices_begin,
                capacity * sizeof(Vertex));
        }
        layout.vertices_begin = replacement;
        layout.vertices_end = replacement + current_count;
        layout.vertices_capacity_end = replacement + new_capacity;
    }

    for (std::size_t index = 0; index < additional_count; ++index) {
        layout.vertices_end[index] = default_vertex();
    }
    layout.vertices_end += additional_count;
}

template <typename Vertex>
void orbis_mesh_release_vertex_storage(OrbisMesh& mesh) {
    auto& layout = mesh_layout<Vertex>(mesh);
    if (layout.vertices_begin != nullptr) {
        engine_deallocate_sized(
            layout.vertices_begin,
            static_cast<std::size_t>(
                layout.vertices_capacity_end - layout.vertices_begin) *
                sizeof(Vertex));
    }
    layout.vertices_begin = nullptr;
    layout.vertices_end = nullptr;
    layout.vertices_capacity_end = nullptr;
}

template <typename Vertex>
void orbis_mesh_rebuild_vertex_buffers(
    OrbisMesh& mesh,
    RenderMeshFormat mesh_format) {
    constexpr const char* kAllocationName = "VBuffer";

    auto& layout = mesh_layout<Vertex>(mesh);
    const auto vertex_count = mesh_vertex_count<Vertex>(mesh);
    if (vertex_count == 0) {
        return;
    }

    const auto byte_count = vertex_count * sizeof(Vertex);
    if (layout.vertex_buffer_capacity != vertex_count) {
        if (g_orbis_render_system != nullptr) {
            orbis_defer_allocation_release(
                *g_orbis_render_system, layout.vertex_buffers[0]);
            orbis_defer_allocation_release(
                *g_orbis_render_system, layout.vertex_buffers[1]);
        }
        layout.vertex_buffers[0] = static_cast<Vertex*>(
            render_allocate_named(byte_count, kAllocationName, 4));
        layout.vertex_buffers[1] =
            (layout.base.vertex_usage_flags & 1U) != 0
            ? static_cast<Vertex*>(
                  render_allocate_named(byte_count, kAllocationName, 4))
            : nullptr;
        layout.vertex_buffer_capacity = vertex_count;
    }

    std::memcpy(
        layout.vertex_buffers[layout.active_vertex_buffer],
        layout.vertices_begin,
        byte_count);

    const auto* format = render_mesh_format_descriptor(mesh_format);
    layout.descriptor_mask = 0;
    for (std::size_t buffer_index = 0; buffer_index < 2; ++buffer_index) {
        if (layout.vertex_buffers[buffer_index] != nullptr) {
            orbis_build_mesh_vertex_descriptors(
                layout.vertex_descriptors[buffer_index],
                layout.vertex_buffers[buffer_index],
                layout.descriptor_mask,
                static_cast<std::uint32_t>(vertex_count),
                *format);
        }
    }
}

template <typename Vertex>
void orbis_mesh_rebuild_index_buffer(OrbisMesh& mesh) {
    constexpr const char* kAllocationName = "IBuffer";

    auto& layout = mesh_layout<Vertex>(mesh);
    const auto& triangles = layout.base.triangles;
    if (triangles.begin == triangles.end) {
        return;
    }

    const auto index_count = static_cast<std::size_t>(
        reinterpret_cast<const std::uint32_t*>(triangles.end) -
        reinterpret_cast<const std::uint32_t*>(triangles.begin));
    const bool use_32_bit_indices =
        mesh_vertex_count<Vertex>(mesh) > 0xFFFF;
    const auto index_format = use_32_bit_indices
        ? OrbisIndexSize::k32Bit
        : OrbisIndexSize::k16Bit;
    const auto index_byte_size = use_32_bit_indices
        ? sizeof(std::uint32_t)
        : sizeof(std::uint16_t);
    const auto byte_count = index_count * index_byte_size;

    if (layout.index_buffer_capacity != index_count ||
        layout.index_format != static_cast<std::uint32_t>(index_format)) {
        if (g_orbis_render_system != nullptr) {
            orbis_defer_allocation_release(
                *g_orbis_render_system, layout.index_buffer);
        }
        layout.index_buffer = render_allocate_named(
            byte_count, kAllocationName, 4);
        layout.index_buffer_capacity = index_count;
        layout.index_format = static_cast<std::uint32_t>(index_format);
    }

    const auto* source =
        reinterpret_cast<const std::uint32_t*>(triangles.begin);
    if (use_32_bit_indices) {
        std::memcpy(layout.index_buffer, source, byte_count);
        return;
    }

    auto* destination = static_cast<std::uint16_t*>(layout.index_buffer);
    for (std::size_t index = 0; index < index_count; ++index) {
        destination[index] = static_cast<std::uint16_t>(source[index]);
    }
}

}  // namespace rb4
