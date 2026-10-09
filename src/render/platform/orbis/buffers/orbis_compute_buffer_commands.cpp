#include "render/platform/orbis/buffers/orbis_compute_buffer_commands.h"

#include <cstddef>
#include <cstdint>

#include "renderps4/buffers/PS4ComputeBuffer.h"
#include "render/platform/orbis/buffers/orbis_compute_buffer_commands_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8EA740.
void orbis_render_context_copy_compute_buffer_count(
    OrbisRenderContext& context,
    const PS4ComputeBuffer& source,
    PS4ComputeBuffer& destination) {
    const auto& descriptor =
        source.ActiveBuffer();
    orbis_render_context_bind_compute_rw_buffer(context, 0, &descriptor);

    orbis_render_context_copy_gds_to_memory(
        context,
        0,
        destination.ActiveStorage(),
        sizeof(std::uint32_t),
        true);

    orbis_render_context_bind_compute_rw_buffer(context, 0, nullptr);
}

}  // namespace rb4
