#include "render/platform/orbis/context/orbis_transient_draw.h"
#include "renderps4/context/PS4Context.h"

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/context/orbis_render_context.h"
#include "render/platform/orbis/meshes/orbis_gnm_mesh_api.h"
#include "renderps4/system/PS4Device.h"

using namespace rb4;

// Reconstructed from eboot.elf at 0x8EA2D0.
void PS4Context::_DrawPrimitivesImpl(RndPrimitive primitive, RndVertexType format, const void* vertices, unsigned long vertex_count) {
    auto& context = *this;
    const auto primitive_type = static_cast<rb4::MeshPrimitiveType>(primitive);
    const auto frame = orbis_render_context_active_frame(context);
    auto& transient = mTransientBuffers[frame][format];
    auto& commands = orbis_active_render_command_context(context);
    const auto first_vertex = transient.Write(vertices, vertex_count);
    transient.Bind(commands);
    orbis_bind_vertex_buffers(
        commands,
        static_cast<std::uint32_t>(kMeshVertexStreamCount),
        static_cast<std::uint32_t>(kInstanceVertexStreamCount),
        gPS4Device->mIdentityInstanceDescs);
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
