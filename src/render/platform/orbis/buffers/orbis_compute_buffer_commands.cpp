#include "render/platform/orbis/buffers/orbis_compute_buffer_commands.h"

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/buffers/orbis_compute_buffer_commands_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8EA740.
void orbis_render_context_copy_compute_buffer_count(
    OrbisRenderContext& context,
    const OrbisComputeBuffer& source,
    OrbisComputeBuffer& destination) {
    const auto& descriptor =
        orbis_compute_buffer_active_descriptor(source);
    orbis_render_context_bind_compute_rw_buffer(context, 0, &descriptor);

    orbis_render_context_copy_gds_to_memory(
        context,
        0,
        orbis_compute_buffer_active_storage(destination),
        sizeof(std::uint32_t),
        true);

    orbis_render_context_bind_compute_rw_buffer(context, 0, nullptr);
}

}  // namespace rb4
