#include "render/core/system/render_system_globals.h"

#include <cstddef>
#include <cstdint>

namespace rb4 {

RenderSystem* g_render_system = nullptr;

namespace {

struct RenderSystemFramePrefix {
    std::uint8_t reserved_0[56];
    RenderFrameOwner* primary_frame_owner;
    std::uint8_t reserved_64[8];
    RenderFrameOwner** frame_owners_begin;
    RenderFrameOwner** frame_owners_end;
    std::uint8_t reserved_88[24];
    RenderFrameOwner* frame_owner;
};

static_assert(
    offsetof(RenderSystemFramePrefix, primary_frame_owner) == 56);
static_assert(
    offsetof(RenderSystemFramePrefix, frame_owners_begin) == 72);
static_assert(offsetof(RenderSystemFramePrefix, frame_owners_end) == 80);
static_assert(offsetof(RenderSystemFramePrefix, frame_owner) == 112);

}  // namespace

RenderSystem* render_system_instance() {
    return g_render_system;
}

RenderFrameOwner* render_system_frame_owner(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    return runtime->frame_owner;
}

RenderFrameOwner& render_system_primary_frame_owner(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    return *runtime->primary_frame_owner;
}

std::size_t render_system_frame_owner_count(const RenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const RenderSystemFramePrefix*>(&system);
    return static_cast<std::size_t>(
        runtime->frame_owners_end - runtime->frame_owners_begin);
}

RenderFrameOwner& render_system_frame_owner_at(
    RenderSystem& system,
    std::size_t index) {
    auto* runtime = reinterpret_cast<RenderSystemFramePrefix*>(&system);
    return *runtime->frame_owners_begin[index];
}

void render_system_publish_instance(RenderSystem& system) {
    g_render_system = &system;
}

void render_system_clear_instance() {
    g_render_system = nullptr;
}

}  // namespace rb4
