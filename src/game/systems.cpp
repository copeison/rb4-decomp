#include "systems.h"

#include "systems_adapters.h"
#include "../render/render_system_runtime.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x402D30.
void game_systems_shutdown(void* context) {
    auto* render_system = game_render_system_instance();
    if (render_system == nullptr || game_systems_shutdown_in_progress()) {
        return;
    }

    game_systems_set_shutdown_in_progress(true);
    game_render_dependents_shutdown(context);
    render_system_shutdown(*render_system);
    game_render_system_delete(*render_system);
    game_clear_render_system_instance();
    game_systems_set_shutdown_in_progress(false);
}

}  // namespace rb4
