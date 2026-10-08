#include "orbis_frame_submit.h"

#include "orbis_frame_submit_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x8D8300.
void orbis_render_system_submit_frame(
    OrbisRenderSystem& system,
    const std::vector<OrbisRenderObject*>& objects) {
    orbis_lock_submission(system);
    orbis_submit_scope_begin(system);

    while (!orbis_submit_token_available(system)) {
        orbis_wait_for_submit_token(system);
    }
    orbis_consume_submit_token(system);

    orbis_submit_scope_end(system);
    if (orbis_frame_is_active(system)) {
        orbis_flush_active_frame(system);
    }

    orbis_submit_primary_frame_owner(system);
    for (auto* object : objects) {
        orbis_finalize_render_object(*object);
    }

    orbis_unlock_submission(system);
}

}  // namespace rb4
