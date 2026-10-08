#include "render/gbuffer/gbuffer_targets.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/targets/render_target_resource_factory.h"
#include "render/core/textures/render_data_format.h"
#include "render/core/textures/render_texture.h"

namespace rb4 {

namespace {

RenderTexture*& target_slot(
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

RenderTexture* target_slot(
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

RenderTexture* reusable_target(
    const RenderTargetResourceBlock* block,
    GBufferTargetKind kind) {
    return block == nullptr
        ? nullptr
        : target_slot(*block, kind);
}

const char* target_name(GBufferTargetKind kind) {
    switch (kind) {
    case GBufferTargetKind::kColor:
        return "GBuffer Color";
    case GBufferTargetKind::kPixelNormals:
        return "GBuffer Pixel Normals";
    case GBufferTargetKind::kVertexNormals:
        return "GBuffer Vertex Normals";
    }
    return "GBuffer";
}

void create_target(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    const RenderTargetResourceBlock* reusable_block,
    GBufferTargetKind kind,
    RenderExtent extent,
    bool register_with_owner) {
    RenderTextureCreationState creation_state{};
    creation_state.values[6] = 1;
    creation_state.values[8] = static_cast<std::uint32_t>(
        render_texture_default_address_mode(31));
    creation_state.values[9] = static_cast<std::uint32_t>(
        render_texture_default_filter_mode(31));
    creation_state.values[10] = 10;
    const auto is_color = kind == GBufferTargetKind::kColor;
    const RenderDataFormatDescriptor format_descriptor{
        32,
        4,
        is_color ? 0U : 1U,
        is_color ? 2U : 1U,
        -1,
    };
    auto* target = render_target_resources_create_texture_2d(
        resources,
        target_name(kind),
        creation_state,
        render_data_format_resolve(format_descriptor, 7),
        extent,
        -1,
        0,
        reusable_target(reusable_block, kind));
    target_slot(block, kind) = target;
    if (register_with_owner) {
        resources.registered_resources_begin[
            resources.registered_resource_count++] = target;
    }
}

void release_target(
    RenderTargetResourceBlock& block,
    GBufferTargetKind kind) {
    auto*& target = target_slot(block, kind);
    if (target != nullptr) {
        render_texture_release_dynamic(*target);
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
