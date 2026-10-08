#include "render/core/debug/render_debug_commands.h"

#include <array>

#include "render/core/capture/screenshot_capture.h"
#include "render/core/debug/render_debug_command_adapters.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_state.h"
#include "render/resources/system/render_resource_manager.h"

namespace rb4 {

namespace {

RenderSystem& render_system() {
    return *render_system_instance();
}

RenderSettings& render_settings() {
    return *render_system_settings(render_system());
}

void toggle(bool& value) {
    value = !value;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6BA880.
void render_command_toggle_vsync() {
    toggle(render_settings().vsync_enabled);
}

// Reconstructed from eboot.elf at 0x6BA8B0.
void render_command_toggle_scene_mask() {
    toggle(render_settings().scene_mask_enabled);
}

// Reconstructed from eboot.elf at 0x6BA8E0.
void render_command_toggle_shadows() {
    toggle(render_settings().shadows_enabled);
}

// Reconstructed from eboot.elf at 0x6BA910.
void render_command_toggle_postproc() {
    toggle(render_settings().postproc_enabled);
}

// Reconstructed from eboot.elf at 0x6BA940.
void render_command_toggle_tonemapping() {
    toggle(render_settings().tonemapping_enabled);
}

// Reconstructed from eboot.elf at 0x6BA970.
void render_command_toggle_vscat() {
    toggle(render_settings().volumetric_scattering_enabled);
}

// Reconstructed from eboot.elf at 0x6BA590.
void render_command_reload_shaders() {
    render_resource_manager_reload_shaders(
        render_system_resource_manager(render_system()));
}

// Reconstructed from eboot.elf at 0x6BAA40.
void render_command_toggle_multithreaded_rendering() {
    toggle(render_settings().multithreaded_rendering_enabled);
}

// Reconstructed from eboot.elf at 0x6BAA70.
void render_command_toggle_async_compute() {
    toggle(render_settings().async_compute_enabled);
}

// Reconstructed from eboot.elf at 0x6BAAA0.
void render_command_toggle_async_copy() {
    toggle(render_settings().async_copy_enabled);
}

// Reconstructed from eboot.elf at 0x6BAAD0.
void render_command_toggle_tiled_light_interpolation() {
    toggle(render_settings().tiled_light_interpolation_enabled);
}

// Reconstructed from eboot.elf at 0x6BAB00.
void render_command_toggle_partial_framerate() {
    auto& settings = render_settings();
    settings.partial_framerate_enabled =
        settings.max_partial_framerate_scenes != 0 &&
        !settings.partial_framerate_enabled;
}

// Reconstructed from eboot.elf at 0x6BAB40.
void render_command_toggle_stereo_optimizations() {
    toggle(render_settings().stereo_optimizations_enabled);
}

// Reconstructed from eboot.elf at 0x6BAB70.
void render_command_toggle_64_bit_light_accum() {
    toggle(render_settings().use_64_bit_light_accum);
}

// Reconstructed from eboot.elf at 0x6BABA0.
void render_command_toggle_hdr() {
    auto& mode = render_system_core_state(render_system())
                     .render_contexts.hdr_output_mode;
    mode = mode == 1 ? 0 : 1;
}

// Reconstructed from eboot.elf at 0x6BABD0.
void render_command_take_screenshot() {
    screenshot_request();
}

// Reconstructed from eboot.elf at 0x6BAC00.
void render_command_cycle_screenshot_resolution() {
    auto& resolution = render_settings().screenshot_resolution;
    const auto next =
        (static_cast<std::uint32_t>(resolution) + 1) % 6;
    resolution = static_cast<ScreenshotResolution>(next);
    static_cast<void>(screenshot_resolution_name(resolution));
}

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
