#include "render/gbuffer/gbuffer_targets.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/core/targets/render_target_resource_adapters.h"
#include "render/gbuffer/gbuffer_target_adapters.h"

namespace rb4 {

namespace {

RenderTarget* reusable_target(
    const RenderTargetResourceBlock* block,
    GBufferTargetKind kind) {
    return block == nullptr
        ? nullptr
        : render_target_resource_block_gbuffer_target(*block, kind);
}

void create_target(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block,
    GBufferTargetKind kind,
    RenderExtent extent,
    bool register_with_owner) {
    render_target_resource_block_gbuffer_target(block, kind) =
        render_target_resources_create_gbuffer_target(
            resources,
            kind,
            extent,
            reusable_target(reusable_block, kind),
            register_with_owner);
}

void release_target(
    RenderTargetResourceBlock& block,
    GBufferTargetKind kind) {
    auto*& target = render_target_resource_block_gbuffer_target(block, kind);
    if (target != nullptr) {
        render_target_release_dynamic(*target);
        target = nullptr;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B3040.
void render_gbuffer_targets_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block) {
    const auto extent = resources.extent;
    const auto register_with_owner = !partial_frame;

    create_target(
        resources,
        block,
        reusable_block,
        GBufferTargetKind::kColor,
        extent,
        register_with_owner);
    create_target(
        resources,
        block,
        reusable_block,
        GBufferTargetKind::kPixelNormals,
        extent,
        register_with_owner);

    const auto& settings =
        *render_system_settings(*render_system_instance());
    if (settings.use_gbuffer_vertex_normals) {
        create_target(
            resources,
            block,
            reusable_block,
            GBufferTargetKind::kVertexNormals,
            extent,
            register_with_owner);
    } else {
        render_target_resource_block_gbuffer_target(
            block, GBufferTargetKind::kVertexNormals) = nullptr;
    }
}

// Reconstructed from the GBuffer portion of eboot.elf at 0x6AFFE0.
void render_gbuffer_targets_release(RenderTargetResourceBlock& block) {
    release_target(block, GBufferTargetKind::kColor);
    release_target(block, GBufferTargetKind::kPixelNormals);
    release_target(block, GBufferTargetKind::kVertexNormals);
}

}  // namespace rb4
