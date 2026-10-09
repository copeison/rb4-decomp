#include "render/core/system/render_epoch.h"

#include "utl/time/Timer.h"
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

void render_system_begin_runtime_epoch(RenderSystem& system) {
    auto& runtime = render_system_core_state(system);
    const auto timing_state = runtime.frame_timing_initialized;
    if ((timing_state & 0x80000000U) != 0) {
        return;
    }

    runtime.frame_timing_initialized = timing_state + 1;
    if (timing_state == 0) {
        runtime.previous_frame_counter = Hmx::Timer::GetCycleCounter();
    }
}

}  // namespace rb4
