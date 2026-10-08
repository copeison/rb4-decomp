#include "render/postprocessing/antialiasing/cmaa_targets.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/postprocessing/antialiasing/cmaa_target_adapters.h"

namespace rb4 {

namespace {

RenderTarget* reusable_target(
    const RenderTargetResources* resources,
    CmaaTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : render_target_resources_cmaa_target(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    CmaaTargetKind kind,
    RenderExtent extent,
    bool use_64_bit_color) {
    render_target_resources_cmaa_target(resources, kind) =
        render_target_resources_create_cmaa_target(
            resources,
            kind,
            extent,
            use_64_bit_color,
            reusable_target(reusable_resources, kind));
}

void release_target(
    RenderTargetResources& resources,
    CmaaTargetKind kind) {
    auto*& target = render_target_resources_cmaa_target(resources, kind);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B1E60.
void render_cmaa_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources) {
    if (!render_system_supports_cmaa()) {
        return;
    }

    const auto extent = resources.extent;
    const auto& settings =
        *render_system_settings(*render_system_instance());

    auto* reusable_color = reusable_target(
        reusable_resources, CmaaTargetKind::kColor);
    if (reusable_color != nullptr) {
        render_target_resources_cmaa_target(
            resources, CmaaTargetKind::kColor) =
            render_target_resources_create_cmaa_target(
                resources,
                CmaaTargetKind::kColor,
                extent,
                settings.use_64_bit_light_accum,
                reusable_color);
    } else {
        render_target_resources_cmaa_target(
            resources, CmaaTargetKind::kColor) = nullptr;
    }

    create_target(
        resources,
        reusable_resources,
        CmaaTargetKind::kEdge0,
        extent,
        false);
    create_target(
        resources,
        reusable_resources,
        CmaaTargetKind::kEdge1,
        extent,
        false);

    const RenderExtent compressed_extent{
        extent.width / 2 > 0 ? extent.width / 2 : 1,
        extent.height / 2 > 0 ? extent.height / 2 : 1,
    };
    create_target(
        resources,
        reusable_resources,
        CmaaTargetKind::kCompressedEdge,
        compressed_extent,
        false);
    render_target_resources_reset_cmaa_state(resources);
}

// Reconstructed from the CMAA portion of eboot.elf at 0x6AFFE0.
void render_cmaa_targets_release(RenderTargetResources& resources) {
    release_target(resources, CmaaTargetKind::kColor);
    release_target(resources, CmaaTargetKind::kEdge0);
    release_target(resources, CmaaTargetKind::kEdge1);
    release_target(resources, CmaaTargetKind::kCompressedEdge);
}

}  // namespace rb4
