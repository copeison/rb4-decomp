#pragma once

#include "render/core/shaders/render_shader.h"

namespace rb4 {

RenderShader* render_system_create_shader(RenderShaderStage stage);
void render_shader_set_base_dispatch(RenderShader& shader);
bool render_shader_initialize_backend(
    RenderShader& shader,
    const RenderShaderBinary& binary);
void render_shader_release_backend(RenderShader& shader);
void render_delete_shader_storage(RenderShader& shader);

}  // namespace rb4
