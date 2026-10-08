#include "render/core/screenshot_capture.h"

#include "render/core/screenshot_capture_adapters.h"

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
    const RenderFrameOwner& owner,
    ScreenshotResolution resolution) {
    switch (resolution) {
    case ScreenshotResolution::kCurrent:
        return render_frame_owner_output_extent(owner);
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
    RenderFrameOwner& owner,
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

    g_screenshot_draw_mode = render_frame_owner_draw_mode(owner);
    g_screenshot_debug_view = render_frame_owner_debug_view(owner);

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
    auto* system = render_system_instance();
    if (system == nullptr) {
        return;
    }

    auto* owner = render_system_active_frame_owner(*system);
    if (owner != nullptr) {
        screenshot_capture_frame(*owner, screenshot_resolution_mode());
    }
}

}  // namespace rb4
