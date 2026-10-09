#pragma once

#include <cstdint>
#include <vector>

namespace rb4 {

const char* render_configured_api_name(const char* platform_name);
std::vector<const char*> render_configured_resolutions(
    const char* platform_name);
std::vector<std::uint32_t> render_configured_supported_platform_ids(
    const char* block_name,
    const char* list_name);

}  // namespace rb4
