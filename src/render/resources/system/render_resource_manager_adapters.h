#pragma once

#include <cstdint>

namespace rb4 {

struct RenderFloatPixel;
struct RenderResourceManager;

void render_resource_manager_initialize(RenderResourceManager& manager);
void render_primary_shader_finalize(void* shader);
RenderFloatPixel render_function_table_sample(
    std::uint32_t function_index,
    float input);
void render_resource_name_destruct(void* name);

}  // namespace rb4
