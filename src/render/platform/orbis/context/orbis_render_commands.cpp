#include "render/platform/orbis/context/orbis_render_commands.h"
#include "renderps4/context/PS4Context.h"

#include <cstdint>

#include "render/platform/orbis/context/orbis_render_commands_adapters.h"

using namespace rb4;

namespace rb4 {

namespace {

constexpr std::uint32_t kDebugMarkerColor = 0xFF0000FF;

}  // namespace

}  // namespace rb4

// Reconstructed from eboot.elf at 0x8EB870.
void PS4Context::_DispatchComputeImpl(unsigned int group_count_x, unsigned int group_count_y, unsigned int group_count_z) {
    auto& context = *this;
    if (orbis_render_context_recording_graphics(context)) {
        orbis_render_context_prepare_graphics_dispatch(context);
        orbis_render_context_dispatch_graphics(
            context, group_count_x, group_count_y, group_count_z);
        orbis_render_context_finish_graphics_dispatch(context);
    } else if (orbis_render_context_recording_compute(context)) {
        orbis_render_context_prepare_compute_dispatch(context);
        orbis_render_context_dispatch_compute(
            context, group_count_x, group_count_y, group_count_z);
    }
}

// Reconstructed from eboot.elf at 0x8EB970.
void PS4Context::_PushMarkerImpl(const char* name) {
    auto& context = *this;
    if (orbis_render_context_recording_graphics(context)) {
        orbis_render_context_push_graphics_debug_marker(
            context, name, kDebugMarkerColor);
    } else if (orbis_render_context_recording_compute(context)) {
        orbis_render_context_push_compute_debug_marker(
            context, name, kDebugMarkerColor);
    }
}

// Reconstructed from eboot.elf at 0x8EB9D0.
void PS4Context::_PopMarkerImpl() {
    auto& context = *this;
    if (orbis_render_context_recording_graphics(context)) {
        orbis_render_context_pop_graphics_debug_marker(context);
    } else if (orbis_render_context_recording_compute(context)) {
        orbis_render_context_pop_compute_debug_marker(context);
    }
}
