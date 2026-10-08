#pragma once

#include <cstdint>

#include "render/core/targets/render_target_resources.h"

namespace rb4 {

enum class CmaaTargetKind : std::uint32_t {
    kColor,
    kEdge0,
    kEdge1,
    kCompressedEdge,
};

void render_cmaa_targets_create(
    RenderTargetResources& resources,
    const RenderTargetResources* reusable_resources);
void render_cmaa_targets_release(RenderTargetResources& resources);

}  // namespace rb4
