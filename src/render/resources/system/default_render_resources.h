#pragma once

#include <array>

#include "render/buffers/RndComputeBuffer.h"
#include "render/resources/camera/default_camera.h"
#include "render/resources/lighting/default_lighting.h"
#include "render/resources/materials/default_materials.h"
#include "render/resources/textures/default_textures.h"

class RndDevice;

namespace rb4 {

// Constructed inline by RndDevice's constructor (0x6BDB30 in this build).
struct DefaultRenderResources {
    ~DefaultRenderResources();  // 0x6BDC20

    RndSceneResource* scene_resource = nullptr;
    DefaultTextureSet textures;
    std::array<RndComputeBuffer*, 2> compute_buffers{};
    DefaultCameraState camera;
    DefaultMaterialSet materials;
    DefaultLightingState lighting;
};

static_assert(sizeof(DefaultRenderResources) == 568);

void render_initialize_default_resources(
    DefaultRenderResources& resources,
    bool initialize_rendering);
void render_poll_default_resources(DefaultRenderResources& resources);
void render_release_default_resources(DefaultRenderResources& resources);

}  // namespace rb4
