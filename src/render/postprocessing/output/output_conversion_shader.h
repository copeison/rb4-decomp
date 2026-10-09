#pragma once

#include <cstddef>
#include <cstdint>

class RndTextureBase;

namespace rb4 {

struct RenderContext;

// 24-byte parameter block for the final output-conversion draw.
struct RenderOutputConversionDrawParameters {
    RndTextureBase* source;
    RndTextureBase* hmd_mask;
    float minimum_intensity;
    bool bt709_to_bt2020;
    bool perceptual_quantizer;
    std::uint8_t reserved_22[2];
};

static_assert(
    offsetof(RenderOutputConversionDrawParameters, minimum_intensity) == 0x10);
static_assert(
    offsetof(RenderOutputConversionDrawParameters, bt709_to_bt2020) == 0x14);
static_assert(sizeof(RenderOutputConversionDrawParameters) == 0x18);

void render_output_conversion_shader_draw(
    void* shader,
    RenderContext& context,
    const RenderOutputConversionDrawParameters& parameters);

}  // namespace rb4
