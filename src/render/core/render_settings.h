#pragma once

#include <cstdint>

#include "render/core/render_frame_owner.h"
#include "render/core/screenshot_capture.h"

namespace rb4 {

enum class RenderQualityLevel : std::uint32_t {
    kLow = 0,
    kMedium = 1,
    kHigh = 2,
    kInvalid = 0xFFFFFFFFu,
};

struct RenderSettings {
    RenderExtent content_resolution{1920, 1080};
    RenderExtent pc_window_resolution{1280, 720};
    bool pc_fullscreen = false;
    std::int32_t vsync_mode = 0;

    bool use_lod = true;
    bool use_gbuffer_vertex_normals = true;
    bool use_64_bit_light_accum = false;
    bool use_40_bit_depth_stencil = true;
    bool use_tiled_lighting = false;

    std::int32_t max_partial_framerate_scenes = 0;
    std::int32_t max_shadow_contrib_buffers = 0;

    RenderExtent output_resolution{1920, 1080};
    bool resolution_overridden = false;
    RenderQualityLevel quality_level = RenderQualityLevel::kMedium;
    bool vsync_enabled = true;

    bool scene_mask_enabled = true;
    bool shadows_enabled = true;
    bool postproc_enabled = true;
    bool tonemapping_enabled = true;
    bool volumetric_scattering_enabled = true;

    std::int64_t first_drawn_scene = -1;
    std::int64_t last_drawn_scene = -1;
    bool multithreaded_rendering_enabled = true;
    bool async_compute_enabled = true;
    bool async_copy_enabled = true;
    bool tiled_light_interpolation_enabled = true;
    bool partial_framerate_enabled = false;
    bool stereo_optimizations_enabled = true;
    ScreenshotResolution screenshot_resolution = ScreenshotResolution::kCurrent;

    std::int32_t max_geo_overdraw = 10;
    std::int32_t max_lighting_overdraw = 20;
    std::int32_t max_light_probe_overdraw = 10;

    bool graphics_api_validation_enabled = false;
    bool break_on_graphics_warning = false;
    bool break_on_graphics_error = true;
    bool graphics_debugger_enabled = false;
    bool graphics_barrier_validation_enabled = false;
    bool print_shader_compilation = false;
    bool print_verbose_shader_compilation = false;
    bool output_shader_intermediates = false;
    bool generate_shader_debug_info = false;
};

void render_settings_initialize(RenderSettings& settings);
bool render_parse_resolution(const char* text, RenderExtent& extent);
const char* render_quality_level_name(RenderQualityLevel level);
RenderQualityLevel render_quality_level_from_name(const char* name);

}  // namespace rb4
