#include "render/platform/orbis/system/orbis_render_system_globals.h"

#include <cstddef>

#include "render/core/system/render_epoch.h"
#include "render/platform/orbis/video/orbis_back_buffer.h"

namespace rb4 {

OrbisRenderSystem* g_orbis_render_system = nullptr;

namespace {

struct OrbisRenderSystemRuntimePrefix {
    std::uint8_t reserved_0[56];
    OrbisRenderContext* render_context;
    bool frame_active;
    std::uint8_t reserved_65[47];
    OrbisBackBuffer* back_buffer;
    std::uint8_t reserved_120[3684];
    std::int32_t video_output_handle;
    std::uint64_t event_queue;
    std::uint8_t reserved_3816[24];
    std::uint64_t submit_token;
    bool submit_thread_running;
};

static_assert(offsetof(OrbisRenderSystemRuntimePrefix, render_context) == 56);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, frame_active) == 64);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, back_buffer) == 112);
static_assert(
    offsetof(OrbisRenderSystemRuntimePrefix, video_output_handle) == 3804);
static_assert(offsetof(OrbisRenderSystemRuntimePrefix, event_queue) == 3808);
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

std::int32_t orbis_video_output_handle(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->video_output_handle;
}

void orbis_set_video_output_handle(
    OrbisRenderSystem& system,
    std::int32_t handle) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->video_output_handle = handle;
}

void render_system_set_render_context(
    OrbisRenderSystem& system,
    OrbisRenderContext& context) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->render_context = &context;
}

void render_system_set_back_buffer(
    OrbisRenderSystem& system,
    OrbisBackBuffer& back_buffer) {
    auto* runtime = reinterpret_cast<OrbisRenderSystemRuntimePrefix*>(&system);
    runtime->back_buffer = &back_buffer;
}

bool orbis_frame_is_active(const OrbisRenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(&system);
    return runtime->frame_active;
}

std::uint64_t orbis_render_system_epoch(const OrbisRenderSystem& system) {
    const auto& base = reinterpret_cast<const RenderSystem&>(system);
    return render_epoch(base);
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
    const auto* runtime =
        reinterpret_cast<const OrbisRenderSystemRuntimePrefix*>(
            g_orbis_render_system);
    return runtime->back_buffer->active_buffer;
}

void orbis_render_system_publish_instance(OrbisRenderSystem& system) {
    g_orbis_render_system = &system;
}

void orbis_render_system_clear_instance() {
    g_orbis_render_system = nullptr;
}

}  // namespace rb4
