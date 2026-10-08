#include "render/resources/system/default_render_resources.h"

#include <new>

#include "render/core/buffers/render_compute_buffer.h"
#include "render/core/system/render_system.h"
#include "render/core/textures/render_texture.h"

namespace rb4 {

namespace {

constexpr const char* kDefaultComputeBufferName = "Default Compute Buffer";
constexpr std::size_t kDefaultRenderResourcesOffset = 1976;

void replace_scene_resource(
    RndSceneResource*& destination,
    RndSceneResource* replacement) {
    if (destination != nullptr) {
        rnd_scene_resource_release(destination);
    }
    destination = replacement;
}

template <typename Texture>
void release_texture(Texture*& texture) {
    if (texture != nullptr) {
        render_texture_release_dynamic(*texture);
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

DefaultRenderResources& render_system_default_resources(
    RenderSystem& system) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<DefaultRenderResources*>(
        bytes + kDefaultRenderResourcesOffset);
}

// Reconstructed from eboot.elf at 0x6BDB30.
void render_construct_default_resources(DefaultRenderResources& resources) {
    new (&resources) DefaultRenderResources{};
}

// Reconstructed from eboot.elf at 0x6BDC20.
void render_destruct_default_resources(DefaultRenderResources& resources) {
    using LightList = std::vector<RndObjectId>;
    resources.lighting.shadowed_spot_lights.~LightList();
    resources.lighting.directional_lights.~LightList();

    if (resources.lighting.resource != nullptr) {
        rnd_scene_resource_release(resources.lighting.resource);
    }
    if (resources.scene_resource != nullptr) {
        rnd_scene_resource_release(resources.scene_resource);
    }
}

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
        const RenderComputeBufferDescriptor descriptor{
            sizeof(std::uint32_t),
            1,
            &index,
            nullptr,
            0,
            0,
            kDefaultComputeBufferName,
        };
        resources.compute_buffers[index] =
            render_create_compute_buffer(descriptor);
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
            render_compute_buffer_release_dynamic(*buffer);
            buffer = nullptr;
        }
    }
}

}  // namespace rb4
