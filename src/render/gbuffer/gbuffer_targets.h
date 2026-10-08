#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

enum class GBufferTargetKind : std::uint32_t {
    kColor,
    kPixelNormals,
    kVertexNormals,
};

void render_gbuffer_targets_create(
    RenderTargetResources& resources,
    RenderTargetResourceBlock& block,
    bool partial_frame,
    const RenderTargetResourceBlock* reusable_block);
void render_gbuffer_targets_release(RenderTargetResourceBlock& block);

}  // namespace rb4
