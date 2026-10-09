#include "render/debug/screenshot_capture.h"

#include "render/debug/screenshot_capture_adapters.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"

namespace rb4 {

namespace {

constexpr const char* kScreenshotTargetName = "Screenshot";
constexpr const char* kScreenshotPath = "../screenshots/screenshot.png";

bool g_screenshot_pending = false;
ScreenshotRenderTarget* g_screenshot_target = nullptr;
Vector2i g_screenshot_extent;
std::uint32_t g_screenshot_draw_mode = 0;
std::uint32_t g_screenshot_debug_view = 0;

Vector2i extent_for_resolution(
    const RndWindow& owner,
    ScreenshotResolution resolution) {
    switch (resolution) {
    case ScreenshotResolution::kCurrent:
        return owner.GetSize();
    case ScreenshotResolution::k720p:
        return {1280, 720};
    case ScreenshotResolution::k1080p:
        return {1920, 1080};
    case ScreenshotResolution::k4k:
        return {3840, 2160};
    case ScreenshotResolution::k8k:
        return {7680, 4320};
    case ScreenshotResolution::k9024x5076:
        return {9024, 5076};
    }
    return {};
}

bool extents_match(Vector2i left, Vector2i right) {
    return left.x == right.x && left.y == right.y;
}

}  // namespace

// Reconstructed from eboot.elf at 0x43B120.
void screenshot_request() {
    g_screenshot_pending = true;
}

// Reconstructed from eboot.elf at 0x43B130.
bool screenshot_capture_pending() {
    return g_screenshot_pending;
}

// Reconstructed from eboot.elf at 0x43AF40.
const char* screenshot_resolution_name(ScreenshotResolution resolution) {
    switch (resolution) {
    case ScreenshotResolution::kCurrent:
        return "Window Dimensions";
    case ScreenshotResolution::k720p:
        return "1280 x 720 (16:9)";
    case ScreenshotResolution::k1080p:
        return "1920 x 1080 (16:9)";
    case ScreenshotResolution::k4k:
        return "3840 x 2160 (16:9)";
    case ScreenshotResolution::k8k:
        return "7680 x 4320 (16:9)";
    case ScreenshotResolution::k9024x5076:
        return "9024 x 5076 (16:9)";
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x43B140.
void screenshot_capture_frame(
    RndWindow& owner,
    ScreenshotResolution resolution,
    std::function<void()> render) {
    const auto extent = extent_for_resolution(owner, resolution);
    const auto shading_mode = owner.GetShadingMode();
    const auto inspection_mode = owner.GetBufferInspectionMode();
    screenshot_capture_to_file(extent, shading_mode, inspection_mode, render);
}

// Reconstructed from eboot.elf at 0x43B240.
void screenshot_capture_to_file(
    const Vector2i& extent,
    std::uint32_t shading_mode,
    std::uint32_t inspection_mode,
    std::function<void()> render) {
    if (extent.x == 0 || extent.y == 0) {
        return;
    }

    g_screenshot_pending = false;
    if (g_screenshot_target == nullptr ||
        !extents_match(g_screenshot_extent, extent)) {
        g_screenshot_target = screenshot_recreate_render_target(
            g_screenshot_target, extent, kScreenshotTargetName);
        g_screenshot_extent = extent;
    }
    if (g_screenshot_target == nullptr) {
        return;
    }

    g_screenshot_draw_mode = shading_mode;
    g_screenshot_debug_view = inspection_mode;

    screenshot_bind_render_target(
        *g_screenshot_target,
        g_screenshot_draw_mode,
        g_screenshot_debug_view);
    render();
    screenshot_submit_render_target(*g_screenshot_target);
    screenshot_copy_render_target_to_readback(*g_screenshot_target);
    screenshot_write_readback_png(kScreenshotPath);
}

}  // namespace rb4
