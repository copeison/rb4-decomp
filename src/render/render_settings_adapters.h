#pragma once

#include <cstdint>

#include "render_settings.h"

namespace rb4 {

struct DataConfig;

const DataConfig* load_data_config(const char* name);
void config_read_bool(
    const DataConfig& config,
    const char* key,
    bool& destination);
void config_read_int32(
    const DataConfig& config,
    const char* key,
    std::int32_t& destination);
void config_read_extent(
    const DataConfig& config,
    const char* key,
    RenderExtent& destination);
const DataConfig* config_find_block(
    const DataConfig& config,
    const char* key);
const char* config_read_string(
    const DataConfig& config,
    const char* key);

std::uint32_t render_quality_level_from_name(const char* name);
bool render_platform_supports_async_compute();
RenderExtent render_platform_default_resolution();
bool command_line_resolution_override(RenderExtent& resolution);
bool render_platform_supports_resolution(RenderExtent resolution);

}  // namespace rb4
