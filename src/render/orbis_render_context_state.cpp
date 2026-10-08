#include "orbis_render_context.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "orbis_render_context_state_adapters.h"
#include "render_runtime_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kColorRenderTargetCount = 8;
constexpr std::int32_t kUnboundTargetKind = -1;
constexpr std::int32_t kPerTargetBlendMode = 11;

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

}  // namespace rb4
