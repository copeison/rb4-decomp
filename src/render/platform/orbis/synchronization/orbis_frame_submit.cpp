#include "render/platform/orbis/synchronization/orbis_frame_submit.h"

#include "render/platform/orbis/video/orbis_back_buffer.h"
#include "render/core/system/render_system_globals.h"
#include "render/platform/orbis/synchronization/orbis_frame_submit_adapters.h"
#include "render/platform/orbis/context/orbis_render_context.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8D8300.
void orbis_render_system_submit_frame(
    OrbisRenderSystem& system,
    const RenderFrameOwnerList& back_buffers) {
    orbis_lock_submission(system);
    orbis_submit_scope_begin(system);

    while (!orbis_submit_token_available(system)) {
        orbis_wait_for_submit_token(system);
    }
    orbis_consume_submit_token(system);

    orbis_submit_scope_end(system);
    auto& base = orbis_render_system_base(system);
    if (render_system_has_pending_frame(base)) {
        render_system_activate_pending_frame(base);
    }

    orbis_render_context_submit_frame(orbis_render_system_context(system));
    for (std::size_t index = 0; index < back_buffers.count; ++index) {
        auto* back_buffer = reinterpret_cast<OrbisBackBuffer*>(
            back_buffers.items[index]);
        orbis_back_buffer_advance(*back_buffer);
    }

    orbis_unlock_submission(system);
}

}  // namespace rb4
