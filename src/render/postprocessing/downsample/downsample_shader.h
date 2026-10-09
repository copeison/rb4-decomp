#pragma once

#include <cstddef>
#include <cstdint>

class RndTextureBase;

namespace rb4 {

struct RenderContext;

// 16-byte parameter block for a downsample draw. The type selects one of the
// HX_DOWNSAMPLE_COLOR_2X/4X and HX_DOWNSAMPLE_BLOOM_2X/4X programs.
struct RenderDownsampleDrawParameters {
    std::int32_t downsample_type;
    bool value_based_bloom;
    std::uint8_t reserved_5[3];
    RndTextureBase* source;
};

static_assert(offsetof(RenderDownsampleDrawParameters, source) == 8);
static_assert(sizeof(RenderDownsampleDrawParameters) == 16);

void render_downsample_shader_draw(
    void* shader,
    RenderContext& context,
    const RenderDownsampleDrawParameters& parameters);

}  // namespace rb4
