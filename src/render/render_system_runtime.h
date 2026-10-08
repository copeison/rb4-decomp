#pragma once

#include "../game/system_init_options.h"

namespace rb4 {

struct RenderSystem;

void render_system_initialize(
    RenderSystem& system,
    const GameSystemInitOptions& options);
void render_system_initialize_builtin_buffers(RenderSystem& system);
void render_system_shutdown(RenderSystem& system);

}  // namespace rb4
