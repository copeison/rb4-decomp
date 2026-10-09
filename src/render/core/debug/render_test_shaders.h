#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct RenderContext;

struct RenderTestPatternDrawParameters {
    float color0[4];
    float color1[4];
    float tile_count[2];
};

static_assert(offsetof(RenderTestPatternDrawParameters, tile_count) == 0x20);
static_assert(sizeof(RenderTestPatternDrawParameters) == 0x28);

struct RenderTestSimpleDrawParameters {
    bool vertex_color;
    bool constant_buffer_color;
    std::uint8_t reserved_2[2];
    float color[4];
};

static_assert(offsetof(RenderTestSimpleDrawParameters, color) == 4);
static_assert(sizeof(RenderTestSimpleDrawParameters) == 0x14);

void render_test_pattern_shader_draw(
    void* shader,
    RenderContext& context,
    const RenderTestPatternDrawParameters& parameters);
void render_test_simple_shader_draw(
    void* shader,
    RenderContext& context,
    const RenderTestSimpleDrawParameters& parameters);

}  // namespace rb4
