#pragma once

#include <cstdint>

namespace rb4 {

struct RenderPrimaryShaderResource;

void render_primary_shader_set_base_dispatch(
    RenderPrimaryShaderResource& shader);
void render_primary_shader_initialize_backend(
    RenderPrimaryShaderResource& shader);
void render_shader_parameter_registry_add(
    void* binding,
    void* registry,
    const void* parameter_name,
    std::uint32_t first_value,
    std::uint32_t last_value);

}  // namespace rb4
