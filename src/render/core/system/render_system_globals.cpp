#include "render/core/system/render_system_globals.h"

#include <cstddef>
#include <cstdint>

namespace rb4 {

RenderSystem* g_render_system = nullptr;

namespace {

struct RenderSystemFramePrefix {
    std::uint8_t reserved_0[112];
    RenderFrameOwner* frame_owner;
};

static_assert(offsetof(RenderSystemFramePrefix, frame_owner) == 112);

}  // namespace

RenderSystem* render_system_instance() {
    return g_render_system;
}

RenderFrameOwner* render_system_frame_owner(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    return runtime->frame_owner;
}

void render_system_publish_instance(RenderSystem& system) {
    g_render_system = &system;
}

void render_system_clear_instance() {
    g_render_system = nullptr;
}

}  // namespace rb4
