#include "render_debug_commands.h"

#include <array>

#include "render_debug_command_adapters.h"

namespace rb4 {

namespace {

struct RenderDebugCommandDefinition {
    const char* name;
    RenderDebugCommandHandler handler;
};

constexpr std::array<RenderDebugCommandDefinition, 24> kRenderDebugCommands = {{
    {"toggle_overlay", render_command_toggle_overlay},
    {"overlay_help", render_command_overlay_help},
    {"reload_shaders", render_command_reload_shaders},
    {"set_resolution", render_command_set_resolution},
    {"set_quality_level", render_command_set_quality_level},
    {"toggle_vsync", render_command_toggle_vsync},
    {"toggle_scene_mask", render_command_toggle_scene_mask},
    {"toggle_shadows", render_command_toggle_shadows},
    {"toggle_postproc", render_command_toggle_postproc},
    {"toggle_tonemapping", render_command_toggle_tonemapping},
    {"toggle_vscat", render_command_toggle_vscat},
    {"set_drawn_scene_range", render_command_set_drawn_scene_range},
    {
        "toggle_multithreaded_rendering",
        render_command_toggle_multithreaded_rendering,
    },
    {"toggle_async_compute", render_command_toggle_async_compute},
    {"toggle_async_copy", render_command_toggle_async_copy},
    {
        "toggle_tiled_light_interpolation",
        render_command_toggle_tiled_light_interpolation,
    },
    {"toggle_partial_framerate", render_command_toggle_partial_framerate},
    {
        "toggle_stereo_optimizations",
        render_command_toggle_stereo_optimizations,
    },
    {"toggle_64_bit_light_accum", render_command_toggle_64_bit_light_accum},
    {"toggle_hdr", render_command_toggle_hdr},
    {"take_screenshot", render_command_take_screenshot},
    {
        "cycle_screenshot_resolution",
        render_command_cycle_screenshot_resolution,
    },
    {"set_shading_mode", render_command_set_shading_mode},
    {
        "set_buffer_inspection_mode",
        render_command_set_buffer_inspection_mode,
    },
}};

}  // namespace

// Reconstructed from eboot.elf at 0x6BB0E0.
void render_register_debug_commands() {
    for (const auto& command : kRenderDebugCommands) {
        register_debug_command(command.name, command.handler);
    }
}

}  // namespace rb4
