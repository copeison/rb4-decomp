#include "render/platform/orbis/context/orbis_render_context.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/context/orbis_render_context_state_adapters.h"
#include "render/core/system/render_runtime_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kColorRenderTargetCount = 8;
constexpr std::int32_t kUnboundTargetKind = -1;
constexpr std::int32_t kPerTargetBlendMode = 11;
constexpr std::array<std::uint8_t, 10> kStencilMasks = {
    0xFF, 0x07, 0x08, 0x10, 0x0F,
    0x1F, 0x20, 0x28, 0x30, 0xC0,
};

std::uint8_t stencil_mask(std::uint32_t index) {
    return index < kStencilMasks.size() ? kStencilMasks[index] : 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x8E8850.
void orbis_render_context_reset_pipeline_state(OrbisRenderContext& context) {
    orbis_render_context_reset_cached_pipeline_state(context);
    orbis_render_context_bind_render_targets(
        context,
        kUnboundTargetKind,
        orbis_default_render_target_binding());
    orbis_render_context_set_blend_mode(
        context,
        RndMaterialBlendMode::kSource,
        orbis_default_blend_configuration());
    orbis_render_context_set_default_raster_state(context);
    orbis_render_context_set_default_depth_stencil_state(context);
    orbis_render_context_disable_stream_output(context);
    orbis_render_context_clear_shader_resources(context);
}

// Reconstructed from eboot.elf at 0x8E8D20.
void orbis_render_context_bind_render_targets(
    OrbisRenderContext& context,
    std::int32_t target_kind,
    const OrbisRenderTargetBinding& binding) {
    std::array<const OrbisGpuRenderTarget*, kColorRenderTargetCount>
        color_targets{};
    const auto color_count = std::min(
        orbis_render_target_color_count(binding), color_targets.size());
    for (std::size_t slot = 0; slot < color_count; ++slot) {
        color_targets[slot] = orbis_resolve_color_render_target(
            binding, target_kind, slot);
    }
    const auto* depth_target =
        orbis_resolve_depth_render_target(binding, target_kind);

    for (std::size_t slot = 0; slot < color_targets.size(); ++slot) {
        orbis_render_context_bind_color_target(
            context, slot, color_targets[slot]);
    }
    orbis_render_context_bind_depth_target(context, depth_target);
    orbis_render_context_set_viewport_and_scissor(
        context, orbis_render_target_viewport(binding));

    bool synchronized = false;
    for (std::size_t slot = 0; slot < color_count; ++slot) {
        if (!orbis_color_render_target_requires_sync(binding, slot)) {
            continue;
        }
        if (!synchronized) {
            orbis_render_context_begin_render_target_sync(context);
        }
        orbis_render_context_prepare_color_target(context, binding, slot);
        synchronized = true;
    }

    if (depth_target != nullptr &&
        orbis_depth_render_target_requires_prepare(binding)) {
        synchronized = orbis_render_context_prepare_depth_target(
                           context, *depth_target, binding) ||
                       synchronized;
    }
    if (synchronized) {
        orbis_render_context_finish_render_target_sync(context);
    }
}

// Reconstructed from eboot.elf at 0x8E92D0.
void orbis_render_context_set_blend_mode(
    OrbisRenderContext& context,
    RndMaterialBlendMode mode,
    const OrbisBlendConfiguration& configuration) {
    if (static_cast<std::int32_t>(mode) == kPerTargetBlendMode) {
        for (std::size_t slot = 0; slot < kColorRenderTargetCount; ++slot) {
            const auto control =
                orbis_build_blend_control(mode, configuration, slot);
            orbis_render_context_set_gnm_blend_control(
                context, slot, control);
        }
        return;
    }

    const auto control = orbis_build_blend_control(mode, configuration, 0);
    for (std::size_t slot = 0; slot < kColorRenderTargetCount; ++slot) {
        orbis_render_context_set_gnm_blend_control(context, slot, control);
    }
}

// Reconstructed from eboot.elf at 0x8E9F60.
void orbis_render_context_set_depth_mode(
    OrbisRenderContext& context,
    std::uint32_t depth_mode) {
    orbis_render_context_cache_depth_mode(context, depth_mode);
    orbis_render_context_apply_depth_stencil_state(context);
}

// Reconstructed from eboot.elf at 0x8EA030.
void orbis_render_context_set_stencil_state(
    OrbisRenderContext& context,
    std::uint32_t stencil_mode,
    std::uint8_t reference,
    std::uint32_t read_mask,
    std::uint32_t write_mask) {
    orbis_render_context_cache_stencil_state(
        context,
        stencil_mode,
        reference,
        stencil_mask(read_mask),
        stencil_mask(write_mask));
    orbis_render_context_apply_depth_stencil_state(context);
}

// Reconstructed from eboot.elf at 0x8EA150.
void orbis_render_context_set_front_face(
    OrbisRenderContext& context,
    bool counter_clockwise) {
    orbis_render_context_cache_front_face(context, counter_clockwise);
    orbis_render_context_apply_primitive_setup(context);
}

// Reconstructed from eboot.elf at 0x8EA1C0.
void orbis_render_context_set_cull_mode(
    OrbisRenderContext& context,
    OrbisCullMode cull_mode) {
    orbis_render_context_cache_cull_mode(context, cull_mode);
    orbis_render_context_apply_primitive_setup(context);
}

// Reconstructed from eboot.elf at 0x8EA230.
void orbis_render_context_set_polygon_fill(
    OrbisRenderContext& context,
    bool enabled) {
    orbis_render_context_cache_polygon_fill(context, enabled);
    orbis_render_context_apply_primitive_setup(context);
}

// Reconstructed from eboot.elf at 0x8E96F0.
void orbis_render_context_set_color_write_mask(
    OrbisRenderContext& context,
    std::uint8_t target_mask,
    OrbisColorWriteMode write_mode) {
    std::uint32_t channel_mask = 0;
    if (write_mode == OrbisColorWriteMode::kRgba) {
        channel_mask = 0xF;
    } else if (write_mode == OrbisColorWriteMode::kRgb) {
        channel_mask = 0x7;
    }

    std::uint32_t gnm_mask = 0;
    for (std::size_t slot = 0; slot < kColorRenderTargetCount; ++slot) {
        if ((target_mask & (1U << slot)) != 0) {
            gnm_mask |= channel_mask << (slot * 4);
        }
    }
    orbis_render_context_set_gnm_render_target_mask(context, gnm_mask);
    orbis_render_context_cache_color_write_mask(
        context, target_mask, write_mode);
}

}  // namespace rb4
