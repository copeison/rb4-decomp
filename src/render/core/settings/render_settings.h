#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/frame/render_frame_owner.h"
#include "render/core/capture/screenshot_capture.h"

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
    std::uint8_t reserved_17[3]{};
    std::int32_t vsync_mode = 0;

    bool use_lod = true;
    bool use_gbuffer_vertex_normals = true;
    bool use_64_bit_light_accum = false;
    bool use_40_bit_depth_stencil = true;
    bool use_tiled_lighting = false;
    std::uint8_t reserved_29[3]{};

    std::int64_t light_tile_size = 32;
    std::int64_t light_tile_depth_slices = 8;
    std::int64_t volumetric_scattering_tile_size = 16;
    std::int64_t max_lights_per_tile = 256;
    std::int64_t max_point_lights = 256;
    std::int64_t max_spot_lights = 32;
    std::int64_t max_directional_lights = 16;
    std::int64_t max_light_probes = 128;
    bool unknown_96 = true;
    std::uint8_t reserved_97[7]{};

    std::int64_t max_partial_framerate_scenes = 0;
    std::int64_t max_shadow_contrib_buffers = 0;
    std::int64_t shadow_soften_tile_size = 16;
    std::int64_t mask_tile_size = 16;

    RenderExtent output_resolution{1920, 1080};
    bool resolution_overridden = false;
    std::uint8_t reserved_145[3]{};
    RenderQualityLevel quality_level = RenderQualityLevel::kMedium;
    bool vsync_enabled = true;

    bool scene_mask_enabled = true;
    bool shadows_enabled = true;
    bool postproc_enabled = true;
    bool tonemapping_enabled = true;
    bool volumetric_scattering_enabled = true;
    std::uint8_t reserved_158[2]{};

    std::int64_t first_drawn_scene = -1;
    std::int64_t last_drawn_scene = -1;
    bool multithreaded_rendering_enabled = true;
    bool async_compute_enabled = true;
    bool async_copy_enabled = true;
    bool tiled_light_interpolation_enabled = true;
    bool partial_framerate_enabled = false;
    bool stereo_optimizations_enabled = true;
    std::uint8_t reserved_182[2]{};
    ScreenshotResolution screenshot_resolution = ScreenshotResolution::kCurrent;
    std::uint8_t reserved_188[4]{};

    std::int64_t max_geo_overdraw = 10;
    std::int64_t max_lighting_overdraw = 20;
    std::int64_t max_light_probe_overdraw = 10;

    bool graphics_api_validation_enabled = false;
    bool break_on_graphics_warning = false;
    bool break_on_graphics_error = true;
    bool graphics_debugger_enabled = false;
    bool graphics_barrier_validation_enabled = false;
    bool print_shader_compilation = false;
    bool print_verbose_shader_compilation = false;
    bool output_shader_intermediates = false;
    bool generate_shader_debug_info = false;
    std::uint8_t reserved_225[7]{};
};

static_assert(offsetof(RenderSettings, vsync_mode) == 20);
static_assert(offsetof(RenderSettings, light_tile_size) == 32);
static_assert(offsetof(RenderSettings, light_tile_depth_slices) == 40);
static_assert(offsetof(RenderSettings, volumetric_scattering_tile_size) == 48);
static_assert(offsetof(RenderSettings, max_lights_per_tile) == 56);
static_assert(offsetof(RenderSettings, max_point_lights) == 64);
static_assert(offsetof(RenderSettings, max_spot_lights) == 72);
static_assert(offsetof(RenderSettings, max_directional_lights) == 80);
static_assert(offsetof(RenderSettings, max_light_probes) == 88);
static_assert(offsetof(RenderSettings, unknown_96) == 96);
static_assert(offsetof(RenderSettings, max_partial_framerate_scenes) == 104);
static_assert(offsetof(RenderSettings, max_shadow_contrib_buffers) == 112);
static_assert(offsetof(RenderSettings, shadow_soften_tile_size) == 120);
static_assert(offsetof(RenderSettings, mask_tile_size) == 128);
static_assert(offsetof(RenderSettings, output_resolution) == 136);
static_assert(offsetof(RenderSettings, quality_level) == 148);
static_assert(offsetof(RenderSettings, vsync_enabled) == 152);
static_assert(offsetof(RenderSettings, first_drawn_scene) == 160);
static_assert(offsetof(RenderSettings, multithreaded_rendering_enabled) == 176);
static_assert(offsetof(RenderSettings, screenshot_resolution) == 184);
static_assert(offsetof(RenderSettings, max_geo_overdraw) == 192);
static_assert(offsetof(RenderSettings, graphics_api_validation_enabled) == 216);
static_assert(offsetof(RenderSettings, generate_shader_debug_info) == 224);
static_assert(sizeof(RenderSettings) == 232);

RenderSettings* render_settings_allocate();
void render_settings_release(RenderSettings* settings);
void render_settings_initialize(RenderSettings& settings);
std::int32_t render_settings_active_vsync_mode(
    const RenderSettings& settings);
bool render_parse_resolution(const char* text, RenderExtent& extent);
const char* render_quality_level_name(RenderQualityLevel level);
RenderQualityLevel render_quality_level_from_name(const char* name);

}  // namespace rb4
