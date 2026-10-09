#pragma once

#include <cstdint>

namespace rb4 {

enum class RenderShaderStage : std::uint32_t {
    kVertex = 0,
    kHull = 1,
    kDomain = 2,
    kGeometry = 3,
    kPixel = 4,
    kCompute = 5,
};

struct RenderContext;
struct RenderShaderBinary;

// Compiled program for one shader stage and permutation. The permutation key
// orders compiled objects for binary-search binding, and the last-bound frame
// records the render-system frame epoch of the most recent bind.
struct RenderShader {
    void* implementation;
    std::uint64_t permutation_key;
    bool initialized;
    std::uint8_t reserved[7];
    std::int64_t last_bound_frame;
    void* metadata;
};

static_assert(sizeof(RenderShader) == 40);

RenderShader* render_create_shader(RenderShaderStage stage);
void render_shader_construct(RenderShader& shader);
void render_shader_destruct(RenderShader& shader);
void render_shader_delete(RenderShader& shader);
bool render_shader_initialize(
    RenderShader& shader,
    std::uint64_t permutation_key,
    const RenderShaderBinary* binary,
    void* metadata);
void render_shader_release(RenderShader& shader);
void render_shader_bind(const RenderShader& shader, RenderContext& context);

}  // namespace rb4
