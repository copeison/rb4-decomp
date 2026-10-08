#include "orbis_transient_draw.h"

#include <cstddef>
#include <cstdint>

#include "orbis_mesh_adapters.h"
#include "orbis_transient_draw_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8EA2D0.
void orbis_render_context_draw_transient(
    OrbisRenderContext& context,
    MeshPrimitiveType primitive_type,
    RenderMeshFormat format,
    const void* vertices,
    std::size_t vertex_count) {
    const auto first_vertex = orbis_transient_vertex_buffer_append(
        context, format, vertices, vertex_count);
    orbis_transient_vertex_buffer_bind(context, format);
    orbis_bind_default_instance_vertex_buffers(context);

    auto& commands = orbis_active_render_command_context(context);
    auto* indices = static_cast<std::uint16_t*>(
        orbis_allocate_embedded_data(
            commands,
            vertex_count * sizeof(std::uint16_t),
            alignof(std::uint32_t)));
    for (std::size_t index = 0; index < vertex_count; ++index) {
        indices[index] = static_cast<std::uint16_t>(first_vertex + index);
    }

    gnm_draw_command_buffer_set_index_size(
        commands, OrbisIndexSize::k16Bit, OrbisCachePolicy::kBypass);
    orbis_set_primitive_type(commands, primitive_type);
    gnmx_prepare_draw(commands);
    gnm_draw_command_buffer_draw_index(
        commands, static_cast<std::uint32_t>(vertex_count), indices);
    gnmx_finish_draw(commands);
}

}  // namespace rb4
