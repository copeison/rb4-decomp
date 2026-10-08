#include "render/platform/orbis/system/orbis_render_system_globals.h"

#include <cstddef>

namespace rb4 {

OrbisRenderSystem* g_orbis_render_system = nullptr;

namespace {

struct OrbisRenderSystemRuntimePrefix {
    std::uint8_t reserved_0[56];
    OrbisRenderContext* render_context;
    bool frame_active;
    std::uint8_t reserved_65[95];
    std::uint64_t frame_epoch;
};

static_assert(offsetof(OrbisRenderSystemRuntimePrefix, render_context) == 56);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, frame_active) == 64);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, frame_epoch) == 160);

}  // namespace

OrbisRenderSystem* orbis_render_system_instance() {
    return g_orbis_render_system;
}

OrbisRenderContext& orbis_render_system_context(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    return *runtime->render_context;
}

bool orbis_frame_is_active(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->frame_active;
}

std::uint64_t orbis_render_system_epoch(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->frame_epoch;
}

void orbis_render_system_publish_instance(OrbisRenderSystem& system) {
    g_orbis_render_system = &system;
}

void orbis_render_system_clear_instance() {
    g_orbis_render_system = nullptr;
}

}  // namespace rb4
