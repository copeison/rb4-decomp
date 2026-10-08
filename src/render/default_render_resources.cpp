#include "default_render_resources.h"

namespace rb4 {

namespace {

constexpr const char* kDefaultComputeBufferName = "Default Compute Buffer";

void replace_scene_resource(
    RndSceneResource*& destination,
    RndSceneResource* replacement) {
    if (destination != nullptr) {
        rnd_scene_resource_release(destination);
    }
    destination = replacement;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6BDCA0.
void render_initialize_default_resources(
    DefaultRenderResources& resources,
    bool initialize_rendering) {
    if (!initialize_rendering && !render_force_default_resources()) {
        return;
    }

    replace_scene_resource(
        resources.scene_resource, rnd_scene_resource_create());
    if (resources.scene_resource == nullptr) {
        return;
    }

    auto* scene = rnd_scene_resource_scene(*resources.scene_resource);
    if (scene == nullptr) {
        return;
    }

    render_create_default_textures(resources.textures);
    for (std::uint32_t index = 0; index < resources.compute_buffers.size();
         ++index) {
        resources.compute_buffers[index] = render_create_default_compute_buffer(
            index, kDefaultComputeBufferName);
    }

    render_create_default_camera(resources.camera, *scene);
    render_create_default_materials(resources.materials, *scene);
    if (!render_load_default_lighting(resources.lighting)) {
        render_create_fallback_default_lighting(resources.lighting, *scene);
    }

    rnd_scene_resource_finalize_contents(*resources.scene_resource);
    rnd_scene_resource_finalize(*resources.scene_resource);
    if (resources.lighting.resource != nullptr) {
        rnd_scene_resource_finalize(*resources.lighting.resource);
    }
}

}  // namespace rb4
