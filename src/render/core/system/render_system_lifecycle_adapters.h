#pragma once

namespace rb4 {

struct RenderSettings;
struct RenderSystem;

void render_system_install_base_vtable(RenderSystem& system);

RenderSettings* render_settings_allocate();
void render_settings_release(RenderSettings* settings);
}  // namespace rb4
