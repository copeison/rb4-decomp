#pragma once

#include <cstdint>

#include "render/debug/screenshot_capture.h"

struct ScreenshotRenderTarget;

ScreenshotRenderTarget* screenshot_recreate_render_target(
    ScreenshotRenderTarget* previous,
    Vector2i extent,
    const char* name);
void screenshot_bind_render_target(
    ScreenshotRenderTarget& target,
    std::uint32_t draw_mode,
    std::uint32_t debug_view);
void screenshot_submit_render_target(ScreenshotRenderTarget& target);
void screenshot_copy_render_target_to_readback(ScreenshotRenderTarget& target);
void screenshot_write_readback_png(const char* path);
