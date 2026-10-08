#include "render/platform/orbis/system/orbis_render_system_globals.h"

#include <cstddef>

#include "render/core/frame/render_frame_owner.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

OrbisRenderSystem* g_orbis_render_system = nullptr;

namespace {

struct OrbisRenderSystemRuntimePrefix {
    std::uint8_t reserved_0[56];
    OrbisRenderContext* render_context;
    bool frame_active;
    std::uint8_t reserved_65[95];
    std::uint64_t frame_epoch;
    std::uint8_t reserved_168[3672];
    std::uint64_t submit_token;
    bool submit_thread_running;
};

static_assert(offsetof(OrbisRenderSystemRuntimePrefix, render_context) == 56);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, frame_active) == 64);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, frame_epoch) == 160);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, submit_token) == 3840);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, submit_thread_running) == 3848);

}  // namespace

OrbisRenderSystem* orbis_render_system_instance() {
    return g_orbis_render_system;
}

RenderSystem& orbis_render_system_base(OrbisRenderSystem& system) {
    return reinterpret_cast<RenderSystem&>(system);
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

bool orbis_submit_token_available(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->submit_token != 0;
}

void orbis_consume_submit_token(OrbisRenderSystem& system) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->submit_token = 0;
}

bool orbis_submit_thread_running(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->submit_thread_running;
}

std::size_t orbis_active_render_frame_index() {
    auto& base = orbis_render_system_base(*g_orbis_render_system);
    return render_frame_owner_active_frame_index(
        *render_system_frame_owner(base));
}

void orbis_render_system_publish_instance(OrbisRenderSystem& system) {
    g_orbis_render_system = &system;
}

void orbis_render_system_clear_instance() {
    g_orbis_render_system = nullptr;
}

}  // namespace rb4
