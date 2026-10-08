#pragma once

#include "game/startup/system_init_options.h"

namespace rb4 {

struct RenderGpuStatBlock;
struct RenderSystem;

void render_resource_manager_initialize(void* state);
void render_resource_manager_finalize(void* state);
void render_resource_manager_shutdown(void* state);
void render_lighting_resources_initialize(void* state);
void render_lighting_resources_shutdown(void* state);
void render_backend_resource_create(void*& resource);
void render_backend_resource_release(void*& resource);
void render_primitive_mesh_set_construct(void* mesh_set);
void render_primitive_mesh_set_destruct(void* mesh_set);
void render_gpu_stat_block_initialize(RenderGpuStatBlock& block);
}  // namespace rb4
