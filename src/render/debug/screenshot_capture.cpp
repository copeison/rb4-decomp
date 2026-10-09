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
RenderExtent g_screenshot_extent;
std::uint32_t g_screenshot_draw_mode = 0;
std::uint32_t g_screenshot_debug_view = 0;

RenderExtent extent_for_resolution(
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

bool extents_match(RenderExtent left, RenderExtent right) {
    return left.width == right.width && left.height == right.height;
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

// Reconstructed from eboot.elf at 0x43B140 and 0x43B240.
void screenshot_capture_frame(
    RndWindow& owner,
    ScreenshotResolution resolution) {
    const auto extent = extent_for_resolution(owner, resolution);
    if (extent.empty()) {
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

    g_screenshot_draw_mode = owner.GetShadingMode();
    g_screenshot_debug_view = owner.GetBufferInspectionMode();

    screenshot_bind_render_target(
        *g_screenshot_target,
        g_screenshot_draw_mode,
        g_screenshot_debug_view);
    screenshot_invoke_render_callback();
    screenshot_submit_render_target(*g_screenshot_target);
    screenshot_copy_render_target_to_readback(*g_screenshot_target);
    screenshot_write_readback_png(kScreenshotPath);
}

void screenshot_capture_current_frame() {
    auto* system = TheRndDevice();
    if (system == nullptr) {
        return;
    }

    auto* owner = system->mMainWindow;
    if (owner != nullptr) {
        screenshot_capture_frame(
            *owner,
            system->mSettings->mScreenshotResolution);
    }
}

}  // namespace rb4
