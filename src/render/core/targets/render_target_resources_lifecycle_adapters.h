#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

struct RenderTexture;

void render_target_resources_set_base_dispatch(
    RenderTargetResources& resources);
void render_target_resources_set_concrete_dispatch(
    RenderTargetResources& resources);
void render_target_resources_bind_source_texture(
    RenderTargetResources& resources,
    RenderTexture& source_texture);
void render_target_resources_resize_blocks(
    RenderTargetResources& resources,
    std::size_t count);
void render_target_resources_propagate_resource_mode(
    RenderTargetResources& resources);
void render_target_resources_release_unclassified_target(
    RenderTargetResources& resources);
void render_target_resources_finish_release(
    RenderTargetResources& resources);

}  // namespace rb4
