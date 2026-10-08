#include "render/resources/default_render_resources.h"

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

void release_texture(RndTextureResource*& texture) {
    if (texture != nullptr) {
        rnd_texture_resource_release(texture);
        texture = nullptr;
    }
}

void release_texture_family(DefaultTextureFamily& family) {
    release_texture(family.texture_1d);
    release_texture(family.texture_2d);
    release_texture(family.texture_3d);
    release_texture(family.texture_cube);
    release_texture(family.texture_array_1d);
    release_texture(family.texture_array_2d);
    release_texture(family.texture_array_cube);
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

// Reconstructed from eboot.elf at 0x6BFA00.
void render_poll_default_resources(DefaultRenderResources& resources) {
    if (resources.scene_resource != nullptr) {
        rnd_scene_resource_poll(*resources.scene_resource);
    }
    if (resources.lighting.resource != nullptr) {
        rnd_scene_resource_poll(*resources.lighting.resource);
    }
}

// Reconstructed from eboot.elf at 0x6BF860.
void render_release_default_resources(DefaultRenderResources& resources) {
    replace_scene_resource(resources.scene_resource, nullptr);
    replace_scene_resource(resources.lighting.resource, nullptr);

    resources.camera = {};
    resources.materials = {};
    resources.lighting.scene_settings = nullptr;
    resources.lighting.probe = nullptr;
    resources.lighting.directional_lights.clear();
    resources.lighting.shadowed_spot_lights.clear();

    for (auto& family : resources.textures.families) {
        release_texture_family(family);
    }
    for (auto*& buffer : resources.compute_buffers) {
        if (buffer != nullptr) {
            rnd_compute_buffer_release(buffer);
            buffer = nullptr;
        }
    }
}

}  // namespace rb4
