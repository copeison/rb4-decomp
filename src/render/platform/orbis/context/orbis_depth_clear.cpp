#include "render/platform/orbis/context/orbis_depth_clear.h"

#include <cstdint>

#include "render/platform/orbis/context/orbis_depth_clear_adapters.h"

namespace rb4 {

namespace {

std::uint32_t replicate_byte(std::uint8_t value) {
    const auto word = static_cast<std::uint32_t>(value);
    return word | (word << 8) | (word << 16) | (word << 24);
}

}  // namespace

// Reconstructed from eboot.elf at 0x8E99E0.
bool orbis_render_context_clear_depth_stencil_target(
    OrbisRenderContext& context,
    const OrbisGpuDepthRenderTarget& target,
    float depth,
    std::uint8_t stencil) {
    if (orbis_depth_target_has_htile(target)) {
        orbis_render_context_flush_depth_metadata(context);

        OrbisDepthClearRange stencil_range = {};
        if (orbis_depth_target_stencil_clear_range(
                target, stencil_range)) {
            orbis_render_context_dispatch_depth_clear(
                context, stencil_range, replicate_byte(stencil));
        }

        const auto htile_range =
            orbis_depth_target_htile_clear_range(target);
        orbis_render_context_dispatch_depth_clear(
            context, htile_range, 0);
        return true;
    }

    orbis_render_context_begin_raster_depth_clear(
        context, depth, stencil);
    orbis_render_context_draw_depth_clear(context);
    orbis_render_context_finish_raster_depth_clear(context);
    return false;
}

// Reconstructed from eboot.elf at 0x8EBDA0.
void orbis_render_context_draw_depth_clear(
    OrbisRenderContext& context) {
    orbis_render_context_bind_depth_clear_shader(context);
    orbis_render_context_set_depth_clear_draw_state(context, false);
    orbis_render_context_unbind_pixel_shader(context);
    orbis_render_context_submit_depth_clear_draw(context);
    orbis_render_context_set_depth_clear_draw_state(context, true);
}

}  // namespace rb4
