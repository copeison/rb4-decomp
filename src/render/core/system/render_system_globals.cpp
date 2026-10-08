#include "render/core/system/render_system_globals.h"

namespace rb4 {

RenderSystem* g_render_system = nullptr;

RenderSystem* render_system_instance() {
    return g_render_system;
}

void render_system_publish_instance(RenderSystem& system) {
    g_render_system = &system;
}

void render_system_clear_instance() {
    g_render_system = nullptr;
}

}  // namespace rb4
