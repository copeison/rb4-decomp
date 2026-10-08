#include "render/postprocessing/antialiasing/cmaa_targets.h"

#include "render/core/platform/render_platform_config.h"
#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_texture_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kCurrentPlatformConfig = 7;

bool render_system_supports_cmaa() {
    auto* system = render_system_instance();
    if (system == nullptr) {
        return false;
    }
    const auto& config = render_system_platform_config_at(
        *system, kCurrentPlatformConfig);
    return (config.feature_flags & 0x10U) != 0;
}

RenderTexture*& target_slot(
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

RenderTexture* target_slot(
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

RenderTexture* reusable_target(
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
    RenderTextureCreationState creation_state{};
    creation_state.values[6] = 1;
    creation_state.values[8] = 1;
    creation_state.values[9] =
        kind == CmaaTargetKind::kColor ? 2U : 1U;
    creation_state.values[10] = 10;

    RenderDataFormatDescriptor format_descriptor{};
    const char* name = "CMAA Edge Buffer";
    if (kind == CmaaTargetKind::kColor) {
        format_descriptor = {
            use_64_bit_color ? 64U : 32U,
            use_64_bit_color ? 4U : 2U,
            2,
            1,
            -1,
        };
        name = "CMAA Color Buffer";
    } else if (kind == CmaaTargetKind::kCompressedEdge) {
        format_descriptor = {32, 4, 3, 1, -1};
        name = "CMAA Compressed Edge Buffer";
    } else {
        format_descriptor = {8, 10, 0, 1, -1};
    }

    auto* target = render_target_resources_create_texture_2d(
        resources,
        name,
        creation_state,
        render_data_format_resolve(format_descriptor, 7),
        extent,
        -1,
        0,
        reusable_target(reusable_resources, kind));
    target_slot(resources, kind) = target;
    resources.registered_resources_begin[
        resources.registered_resource_count++] = target;
}

void release_target(
    RenderTargetResources& resources,
    CmaaTargetKind kind) {
    auto*& target = target_slot(resources, kind);
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
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
        create_target(
            resources,
            reusable_resources,
            CmaaTargetKind::kColor,
            extent,
            settings.use_64_bit_light_accum);
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
