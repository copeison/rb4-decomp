#pragma once

#include <array>

#include "default_camera.h"
#include "default_lighting.h"
#include "default_materials.h"
#include "default_textures.h"

namespace rb4 {

struct DefaultRenderResources {
    RndSceneResource* scene_resource = nullptr;
    DefaultTextureSet textures;
    std::array<RndComputeBuffer*, 2> compute_buffers{};
    DefaultCameraState camera;
    DefaultMaterialSet materials;
    DefaultLightingState lighting;
};

void render_initialize_default_resources(
    DefaultRenderResources& resources,
    bool initialize_rendering);

}  // namespace rb4
