#pragma once

#include <cstddef>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

struct RenderTexture;

void render_target_resources_bind_source_texture(
    RenderTargetResources& resources,
    RenderTexture& source_texture);
void render_target_resources_resize_blocks(
    RenderTargetResources& resources,
    std::size_t count);
RenderTargetResourceBlock& render_target_resources_block_at(
    RenderTargetResources& resources,
    std::size_t index);
const RenderTargetResourceBlock& render_target_resources_block_at(
    const RenderTargetResources& resources,
    std::size_t index);
void render_target_resources_propagate_resource_mode(
    RenderTargetResources& resources);

}  // namespace rb4
