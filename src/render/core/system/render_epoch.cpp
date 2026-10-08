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

std::uint64_t current_render_epoch() {
    const auto* runtime =
        reinterpret_cast<const RenderSystemRuntimePrefix*>(g_render_system);
    return runtime->frame_epoch;
}

}  // namespace rb4
