#include "systems.h"

#include "systems_adapters.h"
#include "../render/default_render_resources.h"
#include "../render/orbis_render_system.h"
#include "../render/render_system_runtime.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x402C30.
void game_systems_initialize(const GameSystemInitOptions& options) {
    auto* orbis_system = orbis_render_system_create();
    auto& render_system = game_render_system_base(*orbis_system);

    game_prepare_render_platform();
    game_warm_render_platform_config(7);
    render_system_initialize(render_system, options);
    render_initialize_default_resources(
        game_default_render_resources(render_system),
        options.initialize_rendering);
    game_render_backend_post_initialize(render_system, options);
    game_render_dependents_initialize(options);
    game_register_cleanup_callback(game_systems_shutdown);
}

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
