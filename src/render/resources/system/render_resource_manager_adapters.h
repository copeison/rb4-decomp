#pragma once

#include <cstdint>

namespace rb4 {

struct RenderResourceManager;

void render_resource_manager_initialize(RenderResourceManager& manager);
void render_primary_shader_initialize_backend(void* shader);
void render_shader_parameter_registry_add(
    void* binding,
    void* registry,
    const void* parameter_name,
    std::uint32_t first_value,
    std::uint32_t last_value);
void render_resource_name_destruct(void* name);

}  // namespace rb4
