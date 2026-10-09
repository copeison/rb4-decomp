#pragma once

#include "render/system/RndDevice.h"

namespace rb4 {

using GameCleanupCallback = void (*)(void* context);

void game_render_backend_post_initialize(
    RndDevice& system,
    const RndInitParams& options);
void game_render_dependents_initialize(const RndInitParams& options);
void game_register_cleanup_callback(GameCleanupCallback callback);

bool game_systems_shutdown_in_progress();
void game_systems_set_shutdown_in_progress(bool in_progress);
void game_render_dependents_shutdown(void* context);
}  // namespace rb4
