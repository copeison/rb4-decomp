#include "render/core/system/render_epoch.h"

#include <cstddef>

#include "render/core/system/render_system_globals.h"

namespace rb4 {

namespace {

struct RenderSystemRuntimePrefix {
    std::uint8_t reserved_0[160];
    std::uint64_t frame_epoch;
};

static_assert(offsetof(RenderSystemRuntimePrefix, frame_epoch) == 160);

}  // namespace

std::uint64_t render_epoch(const RenderSystem& system) {
    const auto* runtime =
        reinterpret_cast<const RenderSystemRuntimePrefix*>(&system);
    return runtime->frame_epoch;
}

std::uint64_t current_render_epoch() {
    return render_epoch(*g_render_system);
}

void advance_render_epoch(RenderSystem& system) {
    auto* runtime = reinterpret_cast<RenderSystemRuntimePrefix*>(&system);
    ++runtime->frame_epoch;
}

}  // namespace rb4
