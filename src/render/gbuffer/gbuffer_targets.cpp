#include "render/gbuffer/gbuffer_targets.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_adapters.h"
#include "render/gbuffer/gbuffer_target_adapters.h"

namespace rb4 {

namespace {

RenderTarget*& target_slot(
    RenderTargetResourceBlock& block,
    GBufferTargetKind kind) {
    switch (kind) {
    case GBufferTargetKind::kColor:
        return block.gbuffer_color;
    case GBufferTargetKind::kPixelNormals:
        return block.gbuffer_pixel_normals;
    case GBufferTargetKind::kVertexNormals:
        return block.gbuffer_vertex_normals;
    }
    return block.gbuffer_color;
}

RenderTarget* target_slot(
    const RenderTargetResourceBlock& block,
    GBufferTargetKind kind) {
    switch (kind) {
    case GBufferTargetKind::kColor:
        return block.gbuffer_color;
    case GBufferTargetKind::kPixelNormals:
        return block.gbuffer_pixel_normals;
    case GBufferTargetKind::kVertexNormals:
        return block.gbuffer_vertex_normals;
    }
    return nullptr;
}

RenderTarget* reusable_target(
    const RenderTargetResourceBlock* block,
    GBufferTargetKind kind) {
    return block == nullptr
        ? nullptr
        : target_slot(*block, kind);
}

void create_target(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block,
    GBufferTargetKind kind,
    RenderExtent extent,
    bool register_with_owner) {
    target_slot(block, kind) =
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
    auto*& target = target_slot(block, kind);
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
        block.gbuffer_vertex_normals = nullptr;
    }
}

// Reconstructed from the GBuffer portion of eboot.elf at 0x6AFFE0.
void render_gbuffer_targets_release(RenderTargetResourceBlock& block) {
    release_target(block, GBufferTargetKind::kColor);
    release_target(block, GBufferTargetKind::kPixelNormals);
    release_target(block, GBufferTargetKind::kVertexNormals);
}

}  // namespace rb4
