#pragma once

#include <cstdint>
#include <functional>

#include "render/system/RndWindow.h"

namespace rb4 {

enum class ScreenshotResolution : std::uint32_t {
    kCurrent = 0,
    k720p = 1,
    k1080p = 2,
    k4k = 3,
    k8k = 4,
    k9024x5076 = 5,
};

void screenshot_request();
bool screenshot_capture_pending();
const char* screenshot_resolution_name(ScreenshotResolution resolution);
// Captures one frame of the window at the chosen resolution, calling render
// to draw it into the screenshot target.
void screenshot_capture_frame(
    RndWindow& owner,
    ScreenshotResolution resolution,
    std::function<void()> render);  // 0x43B140
void screenshot_capture_to_file(
    const Vector2i& extent,
    std::uint32_t shading_mode,
    std::uint32_t inspection_mode,
    std::function<void()> render);  // 0x43B240

}  // namespace rb4
