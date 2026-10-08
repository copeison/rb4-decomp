#pragma once

#include <cstdint>

#include "render/core/settings/render_settings.h"

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

}  // namespace rb4
