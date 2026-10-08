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

struct RenderShaderBinary;

struct RenderShader {
    void* implementation;
    void* owner;
    bool initialized;
    std::uint8_t reserved[7];
    std::int64_t variant_index;
    void* metadata;
};

static_assert(sizeof(RenderShader) == 40);

RenderShader* render_create_shader(RenderShaderStage stage);
void render_shader_construct(RenderShader& shader);
void render_shader_destruct(RenderShader& shader);
void render_shader_delete(RenderShader& shader);
bool render_shader_initialize(
    RenderShader& shader,
    void* owner,
    const RenderShaderBinary* binary,
    void* metadata);
void render_shader_release(RenderShader& shader);

}  // namespace rb4
