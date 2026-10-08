#include "render/postprocessing/antialiasing/cmaa_targets.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/postprocessing/antialiasing/cmaa_target_adapters.h"

namespace rb4 {

namespace {

RenderTarget*& target_slot(
    RenderTargetResources& resources,
    CmaaTargetKind kind) {
    switch (kind) {
    case CmaaTargetKind::kColor:
        return resources.cmaa_color;
    case CmaaTargetKind::kEdge0:
        return resources.cmaa_edges[0];
    case CmaaTargetKind::kEdge1:
        return resources.cmaa_edges[1];
    case CmaaTargetKind::kCompressedEdge:
        return resources.cmaa_compressed_edges;
    }
    return resources.cmaa_color;
}

RenderTarget* target_slot(
    const RenderTargetResources& resources,
    CmaaTargetKind kind) {
    switch (kind) {
    case CmaaTargetKind::kColor:
        return resources.cmaa_color;
    case CmaaTargetKind::kEdge0:
        return resources.cmaa_edges[0];
    case CmaaTargetKind::kEdge1:
        return resources.cmaa_edges[1];
    case CmaaTargetKind::kCompressedEdge:
        return resources.cmaa_compressed_edges;
    }
    return nullptr;
}

RenderTarget* reusable_target(
    const RenderTargetResources* resources,
    CmaaTargetKind kind) {
    return resources == nullptr
        ? nullptr
        : target_slot(*resources, kind);
}

void create_target(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources,
    CmaaTargetKind kind,
    RenderExtent extent,
    bool use_64_bit_color) {
    target_slot(resources, kind) =
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
    auto*& target = target_slot(resources, kind);
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
        resources.cmaa_color =
            render_target_resources_create_cmaa_target(
                resources,
                CmaaTargetKind::kColor,
                extent,
                settings.use_64_bit_light_accum,
                reusable_color);
    } else {
        resources.cmaa_color = nullptr;
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
    resources.cmaa_state = 0;
}

// Reconstructed from the CMAA portion of eboot.elf at 0x6AFFE0.
void render_cmaa_targets_release(RenderTargetResources& resources) {
    release_target(resources, CmaaTargetKind::kColor);
    release_target(resources, CmaaTargetKind::kEdge0);
    release_target(resources, CmaaTargetKind::kEdge1);
    release_target(resources, CmaaTargetKind::kCompressedEdge);
}

}  // namespace rb4
