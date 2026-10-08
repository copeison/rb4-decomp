#include "render/core/system/render_epoch.h"

#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"

namespace rb4 {

std::uint64_t render_epoch(const RenderSystem& system) {
    return render_system_core_state(system).frame_epoch;
}

std::uint64_t current_render_epoch() {
    return render_epoch(*g_render_system);
}

void advance_render_epoch(RenderSystem& system) {
    ++render_system_core_state(system).frame_epoch;
}

}  // namespace rb4
