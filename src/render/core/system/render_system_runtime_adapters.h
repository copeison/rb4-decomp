#pragma once

#include "game/startup/system_init_options.h"

namespace rb4 {

struct RenderSystem;

void render_system_resource_manager_initialize(RenderSystem& system);
void render_system_resource_manager_finalize(RenderSystem& system);
void render_system_backend_resources_initialize(RenderSystem& system);

void render_system_backend_resources_shutdown(RenderSystem& system);
}  // namespace rb4
