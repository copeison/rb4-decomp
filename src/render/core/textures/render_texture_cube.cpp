#include "render/core/textures/render_texture_cube.h"

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/core/textures/render_texture_cube_adapters.h"
#include "render/core/textures/render_texture_mip_chain.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x68CC00.
void render_texture_cube_state_construct(
    RenderTextureCubeState& cube,
    const RenderTextureCubeDescriptorState& descriptor,
    bool has_source_data) {
    for (std::size_t index = 0; index < 6; ++index) {
        render_texture_mip_chain_construct(
            cube.faces[index], descriptor.faces[index], has_source_data);
    }
}

void render_texture_cube_state_destruct(RenderTextureCubeState& cube) {
    for (std::size_t index = 6; index != 0; --index) {
        render_texture_mip_chain_destruct(cube.faces[index - 1]);
    }
}

// Reconstructed from eboot.elf at 0x68D2C0 and 0x68D320.
bool render_texture_cube_prepare_descriptor(
    const RenderTextureCubeDescriptorState& cube) {
    const auto& first = cube.faces[0].fields;
    if (first.width == 0 || first.height == 0 ||
        first.width != first.height || first.depth != 1) {
        return false;
    }

    const auto expected_levels = render_texture_mip_chain_level_count(
        reinterpret_cast<const RenderTextureMipChainState&>(cube.faces[0]));
    for (std::size_t index = 1; index < 6; ++index) {
        const auto& face = cube.faces[index].fields;
        if (face.width != first.width || face.height != first.height ||
            face.depth != first.depth ||
            face.data_format != first.data_format ||
            render_texture_mip_chain_level_count(
                reinterpret_cast<const RenderTextureMipChainState&>(
                    cube.faces[index])) != expected_levels) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x6A1030.
void render_texture_cube_descriptor_construct(
    RenderTextureCubeDescriptor& descriptor) {
    render_texture_descriptor_construct(descriptor.texture_state);
    for (auto& face : descriptor.cube.faces) {
        render_texture_mip_chain_descriptor_construct(face);
    }
    descriptor.texture_state.descriptor_type = 3;
}

// Reconstructed from eboot.elf at 0x6A0DA0.
void render_texture_cube_construct(
    RenderTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_cube_set_base_dispatch(texture);

    texture.descriptor_state = descriptor.texture_state;
    render_texture_cube_state_construct(
        texture.cube,
        descriptor.cube,
        render_texture_descriptor_has_source_data(descriptor.texture_state));

    texture.resource_index = 2;
    const auto& first_face = texture.cube.faces[0].fields;
    texture.descriptor_state.data_format = first_face.data_format;
    texture.descriptor_state.width = first_face.width;
    texture.descriptor_state.height = first_face.height;
    texture.descriptor_state.depth = first_face.depth;
    render_texture_apply_descriptor_state(
        texture, texture.descriptor_state);
}

// Reconstructed from eboot.elf at 0x6A0D10.
RenderTextureCube* render_create_texture_cube(
    RenderTextureCubeDescriptor& descriptor,
    RenderTextureCube* reusable_texture) {
    render_texture_resolve_descriptor_fields(
        &descriptor.texture_state.usage_type,
        3,
        descriptor.texture_state.creation_state.values,
        -1);
    render_texture_cube_prepare_descriptor(descriptor.cube);

    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = render_factory_create_texture_cube(factory, descriptor);
    const auto& settings = *render_system_settings(*render_system_instance());
    const auto deferred_usage = static_cast<RenderTextureUsage>(10);
    if (texture->usage_type != deferred_usage ||
        (texture->flags & 2U) != 0 ||
        !settings.use_tiled_lighting) {
        render_texture_initialize_backend(*texture, reusable_texture);
    }
    return texture;
}

// Reconstructed from eboot.elf at 0x6A0EA0.
void render_texture_cube_destruct(RenderTextureCube& texture) {
    render_texture_cube_set_base_dispatch(texture);
    render_texture_cube_state_destruct(texture.cube);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x6A0F10.
void render_texture_cube_delete(RenderTextureCube& texture) {
    render_texture_cube_destruct(texture);
    render_delete_texture_cube_storage(texture);
}

}  // namespace rb4
