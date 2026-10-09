#include "render/core/settings/render_settings.h"

#include <cctype>
#include <cstdlib>

#include "utl/options/Option.h"
#include "os/memory/MemMgr.h"
#include "render/core/platform/render_platform_config.h"
#include "render/core/settings/render_settings_adapters.h"
#include "render/core/system/render_system_globals.h"

namespace rb4 {

namespace {

constexpr std::size_t kCurrentPlatformConfigIndex = 7;
constexpr std::uint32_t kAsyncComputeFeature = 0x10;

const RenderPlatformConfig& current_platform_config() {
    return render_system_platform_config_at(
        *render_system_instance(),
        kCurrentPlatformConfigIndex);
}

bool platform_supports_async_compute() {
    return (current_platform_config().feature_flags & kAsyncComputeFeature) != 0;
}

RenderExtent platform_default_resolution() {
    return current_platform_config().resolutions.back();
}

bool platform_supports_resolution(RenderExtent resolution) {
    for (const auto& supported : current_platform_config().resolutions) {
        if (supported.width == resolution.width &&
            supported.height == resolution.height) {
            return true;
        }
    }
    return false;
}

bool equals_ignore_ascii_case(const char* left, const char* right) {
    while (*left != '\0' && *right != '\0') {
        const auto left_char = static_cast<unsigned char>(*left++);
        const auto right_char = static_cast<unsigned char>(*right++);
        if (std::tolower(left_char) != std::tolower(right_char)) {
            return false;
        }
    }
    return *left == *right;
}

void read_sign_extended_int32(
    const DataConfig& config,
    const char* key,
    std::int64_t& destination) {
    auto value = static_cast<std::int32_t>(destination);
    config_read_int32(config, key, value);
    destination = value;
}

void read_validation_settings(
    RenderSettings& settings,
    const DataConfig& root) {
    if (const auto* validation =
            config_find_block(root, "graphics_api_validation")) {
        config_read_bool(
            *validation, "enabled", settings.graphics_api_validation_enabled);
        config_read_bool(
            *validation, "break_on_warning", settings.break_on_graphics_warning);
        config_read_bool(
            *validation, "break_on_error", settings.break_on_graphics_error);
    }

    if (const auto* debugger = config_find_block(root, "graphics_debugger")) {
        config_read_bool(
            *debugger, "enabled", settings.graphics_debugger_enabled);
    }
    if (const auto* barrier =
            config_find_block(root, "graphics_barrier_validation")) {
        config_read_bool(
            *barrier, "enabled", settings.graphics_barrier_validation_enabled);
    }
}

void read_shader_compilation_settings(
    RenderSettings& settings,
    const DataConfig& root) {
    const auto* shader = config_find_block(root, "shader_compilation");
    if (shader == nullptr) {
        return;
    }

    config_read_bool(*shader, "print", settings.print_shader_compilation);
    config_read_bool(
        *shader, "print_verbose", settings.print_verbose_shader_compilation);
    config_read_bool(
        *shader, "output_intermediates", settings.output_shader_intermediates);
    config_read_bool(
        *shader, "generate_debug_info", settings.generate_shader_debug_info);
}

void read_render_config(RenderSettings& settings, const DataConfig& config) {
    config_read_extent(config, "content_resolution", settings.content_resolution);
    config_read_bool(config, "pc_init_fullscreen", settings.pc_fullscreen);
    config_read_extent(
        config, "pc_init_window_resolution", settings.pc_window_resolution);
    config_read_int32(config, "vsync_mode", settings.vsync_mode);
    config_read_bool(config, "use_lod", settings.use_lod);
    config_read_bool(
        config,
        "use_gbuffer_vertex_normals",
        settings.use_gbuffer_vertex_normals);
    config_read_bool(
        config, "use_64_bit_light_accum", settings.use_64_bit_light_accum);
    config_read_bool(
        config, "use_40_bit_depth_stencil", settings.use_40_bit_depth_stencil);
    config_read_bool(config, "use_tiled_lighting", settings.use_tiled_lighting);
    read_sign_extended_int32(
        config,
        "max_partial_framerate_scenes",
        settings.max_partial_framerate_scenes);
    read_sign_extended_int32(
        config,
        "max_shadow_contrib_buffers",
        settings.max_shadow_contrib_buffers);

    if (const auto* quality_name = config_read_string(config, "quality_level")) {
        settings.quality_level = render_quality_level_from_name(quality_name);
    }
    config_read_bool(
        config, "scene_mask_enabled", settings.scene_mask_enabled);
    config_read_bool(
        config,
        "multi_threaded_rendering_enabled",
        settings.multithreaded_rendering_enabled);
    config_read_bool(
        config, "async_compute_enabled", settings.async_compute_enabled);
    read_sign_extended_int32(
        config, "max_geo_overdraw", settings.max_geo_overdraw);
    read_sign_extended_int32(
        config, "max_lighting_overdraw", settings.max_lighting_overdraw);
    read_sign_extended_int32(
        config,
        "max_light_probe_overdraw",
        settings.max_light_probe_overdraw);

    read_validation_settings(settings, config);
    read_shader_compilation_settings(settings, config);
}

void apply_platform_limits(RenderSettings& settings) {
    settings.partial_framerate_enabled =
        settings.max_partial_framerate_scenes != 0;

    if (platform_supports_async_compute()) {
        if (settings.async_compute_enabled) {
            settings.multithreaded_rendering_enabled = false;
        }
    } else {
        settings.async_compute_enabled = false;
        settings.use_tiled_lighting = false;
    }
}

}  // namespace

RenderSettings* render_settings_allocate() {
    return static_cast<RenderSettings*>(operator new(sizeof(RenderSettings)));
}

void render_settings_release(RenderSettings* settings) {
    if (settings != nullptr) {
        MemFree(settings);
    }
}

std::int32_t render_settings_active_vsync_mode(
    const RenderSettings& settings) {
    return settings.vsync_enabled ? settings.vsync_mode : 0;
}

// Reconstructed from eboot.elf at 0x441940.
bool render_parse_resolution(const char* text, RenderExtent& extent) {
    if (text == nullptr) {
        return false;
    }

    char* separator = nullptr;
    const auto first_value = std::strtol(text, &separator, 0);
    if (first_value <= 0) {
        return false;
    }

    if (*separator != 'x') {
        extent.height = static_cast<std::uint32_t>(first_value);
        extent.width = 16 * extent.height / 9;
        return true;
    }

    const auto second_value = std::strtol(separator + 1, nullptr, 0);
    if (second_value <= 0) {
        return false;
    }

    extent.width = static_cast<std::uint32_t>(first_value);
    extent.height = static_cast<std::uint32_t>(second_value);
    return true;
}

// Reconstructed from eboot.elf at 0x442520.
const char* render_quality_level_name(RenderQualityLevel level) {
    switch (level) {
    case RenderQualityLevel::kLow:
        return "Low";
    case RenderQualityLevel::kMedium:
        return "Medium";
    case RenderQualityLevel::kHigh:
        return "High";
    case RenderQualityLevel::kInvalid:
        return nullptr;
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x442540.
RenderQualityLevel render_quality_level_from_name(const char* name) {
    if (name != nullptr) {
        if (equals_ignore_ascii_case(name, "Low")) {
            return RenderQualityLevel::kLow;
        }
        if (equals_ignore_ascii_case(name, "Medium")) {
            return RenderQualityLevel::kMedium;
        }
        if (equals_ignore_ascii_case(name, "High")) {
            return RenderQualityLevel::kHigh;
        }
    }
    return RenderQualityLevel::kInvalid;
}

// Reconstructed from eboot.elf at 0x6BB470.
void render_settings_initialize(RenderSettings& settings) {
    settings = {};
    if (const auto* config = load_data_config("rnd")) {
        read_render_config(settings, *config);
    }
    apply_platform_limits(settings);

    settings.output_resolution = platform_default_resolution();
    if (const auto* override_text =
            OptionStr(gOptionArgs, "resolution", nullptr)) {
        RenderExtent override_resolution{};
        if (render_parse_resolution(override_text, override_resolution) &&
            platform_supports_resolution(settings.output_resolution)) {
            settings.output_resolution = override_resolution;
            settings.pc_window_resolution = override_resolution;
            settings.resolution_overridden = true;
        }
    }
}

}  // namespace rb4
