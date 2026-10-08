#pragma once

#include <cstdint>

#include "render/core/render_frame_owner.h"

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
void screenshot_capture_frame(
    RenderFrameOwner& owner,
    ScreenshotResolution resolution);
void screenshot_capture_current_frame();

}  // namespace rb4
