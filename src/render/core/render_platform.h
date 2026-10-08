#pragma once

#include <cstdint>

namespace rb4 {

enum class RenderPlatform : std::uint32_t {
    kUnknown0 = 0,
    kUnknown1 = 1,
    kUnknown2 = 2,
    kPc = 3,
    kUnknown4 = 4,
    kXboxOne = 5,
    kUnknown6 = 6,
    kPlayStation4 = 7,
    kAndroid = 8,
    kIos = 9,
    kMacOs = 10,
    kTvOs = 11,
    kSwitch = 12,
};

enum class RenderApi : std::uint32_t {
    kNull = 0,
    kDirect3D11 = 1,
    kPlayStation4 = 2,
    kMetal = 3,
    kVulkan = 4,
    kSwitch = 5,
    kOpenGles3 = 6,
};

const char* render_platform_name(RenderPlatform platform);
const char* render_api_name(RenderApi api);
RenderApi render_api_for_platform(RenderPlatform platform);
RenderApi orbis_render_api();

}  // namespace rb4
