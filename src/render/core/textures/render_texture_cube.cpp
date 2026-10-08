#include "render/core/textures/render_texture_cube.h"

#include <cstddef>
#include <cstring>

#include "render/core/settings/render_settings.h"
#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/core/textures/render_texture_cube_adapters.h"

namespace rb4 {

namespace {

template <typename T>
T read_value(const std::uint8_t* source, std::size_t offset) {
    T value;
    std::memcpy(&value, source + offset, sizeof(value));
    return value;
}

template <typename T>
void write_value(std::uint8_t* destination, std::size_t offset, T value) {
    std::memcpy(destination + offset, &value, sizeof(value));
}

}  // namespace

// Reconstructed from eboot.elf at 0x6A0DA0.
void render_texture_cube_construct(
    RenderTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_cube_set_base_dispatch(texture);

    std::memcpy(
        texture.descriptor_state,
        &descriptor.texture_state,
        sizeof(texture.descriptor_state));
    render_texture_cube_state_construct(
        texture.cube,
        descriptor.cube,
        render_texture_descriptor_has_source_data(descriptor.texture_state));

    texture.resource_index = 2;
    const auto& first_face = texture.cube.faces[0].storage;
    write_value(
        texture.descriptor_state,
        92,
        read_value<std::uint32_t>(first_face, 20));
    write_value(
        texture.descriptor_state,
        96,
        read_value<std::uint64_t>(first_face, 8));
    write_value(
        texture.descriptor_state,
        104,
        read_value<std::uint32_t>(first_face, 16));
    std::memcpy(
        &texture.descriptor_type,
        texture.descriptor_state,
        sizeof(texture.descriptor_state));
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
