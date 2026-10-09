#include "game/systems/systems.h"

#include "game/systems/systems_adapters.h"
#include "render/resources/system/default_render_resources.h"
#include "render/core/platform/render_platform.h"
#include "render/system/RndDevice.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x402C30.
void game_systems_initialize(const RndInitParams& options) {
    auto& render_system = *Rnd::PlatformCreateDevice();

    (void)orbis_render_api();
    (void)render_api_for_platform(RenderPlatform::kPlayStation4);
    render_system.Init(options);
    render_initialize_default_resources(
        render_system.mDefaults,
        options.mInitRendering);
    game_render_backend_post_initialize(render_system, options);
    game_render_dependents_initialize(options);
    game_register_cleanup_callback(game_systems_shutdown);
}

// Reconstructed from eboot.elf at 0x402D30.
void game_systems_shutdown(void* context) {
    auto* render_system = TheRndDevice();
    if (render_system == nullptr || game_systems_shutdown_in_progress()) {
        return;
    }

    game_systems_set_shutdown_in_progress(true);
    game_render_dependents_shutdown(context);
    render_system->Terminate();
    delete render_system;
    gRndDevice = nullptr;
    game_systems_set_shutdown_in_progress(false);
}

}  // namespace rb4
