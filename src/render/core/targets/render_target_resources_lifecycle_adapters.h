#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

struct RenderTexture;

void render_target_resources_set_base_dispatch(
    RenderTargetResources& resources);
RenderTexture*& render_target_resources_source_texture(
    RenderTargetResources& resources);
void render_target_resources_bind_source_texture(
    RenderTargetResources& resources,
    RenderTexture& source_texture);
void render_target_resources_resize_blocks(
    RenderTargetResources& resources,
    std::size_t count);
std::size_t render_target_resources_block_count(
    const RenderTargetResources& resources);
RenderTargetResourceBlock& render_target_resources_block_at(
    RenderTargetResources& resources,
    std::size_t index);
const RenderTargetResourceBlock& render_target_resources_block_at(
    const RenderTargetResources& resources,
    std::size_t index);
void render_target_resources_propagate_resource_mode(
    RenderTargetResources& resources);
std::int32_t& render_target_resources_resource_mode(
    RenderTargetResources& resources);
std::size_t& render_target_resources_active_block_index(
    RenderTargetResources& resources);
std::int64_t& render_target_resources_active_scene_context(
    RenderTargetResources& resources);
void render_target_resources_release_unclassified_target(
    RenderTargetResources& resources);
void render_target_resources_finish_release(
    RenderTargetResources& resources);

}  // namespace rb4
