#pragma once

#include <cstdint>

namespace rb4 {

struct RenderPrimaryShaderResource;

void render_primary_shader_set_base_dispatch(
    RenderPrimaryShaderResource& shader);
void render_primary_shader_initialize_backend(
    RenderPrimaryShaderResource& shader);
}  // namespace rb4
