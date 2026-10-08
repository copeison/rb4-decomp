#include "render/platform/orbis/system/orbis_render_system_globals.h"

namespace rb4 {

OrbisRenderSystem* g_orbis_render_system = nullptr;

OrbisRenderSystem* orbis_render_system_instance() {
    return g_orbis_render_system;
}

void orbis_render_system_publish_instance(OrbisRenderSystem& system) {
    g_orbis_render_system = &system;
}

void orbis_render_system_clear_instance() {
    g_orbis_render_system = nullptr;
}

}  // namespace rb4
