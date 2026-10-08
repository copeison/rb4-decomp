#pragma once

namespace rb4 {

struct RenderResourceListNode;
struct RenderPrimaryShaderResource;

RenderPrimaryShaderResource& render_primary_shader_from_link(
    RenderResourceListNode& link);

void render_primary_shader_construct(RenderPrimaryShaderResource& shader);
void render_primary_shader_destruct(RenderPrimaryShaderResource& shader);
void render_primary_shader_prepare(RenderPrimaryShaderResource& shader);
void render_primary_shader_finalize(RenderPrimaryShaderResource& shader);
void render_primary_shader_clear_compiled_objects(
    RenderPrimaryShaderResource& shader);

}  // namespace rb4
