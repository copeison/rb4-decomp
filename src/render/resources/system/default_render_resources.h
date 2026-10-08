#pragma once

#include <array>

#include "render/core/buffers/render_compute_buffer.h"
#include "render/resources/camera/default_camera.h"
#include "render/resources/lighting/default_lighting.h"
#include "render/resources/materials/default_materials.h"
#include "render/resources/textures/default_textures.h"

namespace rb4 {

struct RenderSystem;

struct DefaultRenderResources {
    RndSceneResource* scene_resource = nullptr;
    DefaultTextureSet textures;
    std::array<RenderComputeBuffer*, 2> compute_buffers{};
    DefaultCameraState camera;
    DefaultMaterialSet materials;
    DefaultLightingState lighting;
};

static_assert(sizeof(DefaultRenderResources) == 568);

DefaultRenderResources& render_system_default_resources(
    RenderSystem& system);

void render_initialize_default_resources(
    DefaultRenderResources& resources,
    bool initialize_rendering);
void render_poll_default_resources(DefaultRenderResources& resources);
void render_release_default_resources(DefaultRenderResources& resources);

}  // namespace rb4
