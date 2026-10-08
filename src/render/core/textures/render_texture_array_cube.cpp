#include "render/core/textures/render_texture_array_cube.h"

#include <cstddef>
#include <cstring>

#include "render/core/system/render_factory.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/textures/render_texture_adapters.h"
#include "render/core/textures/render_texture_array_cube_adapters.h"

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

// Reconstructed from eboot.elf at 0x69AAC0.
void render_texture_array_cube_construct(
    RenderTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_array_cube_set_base_dispatch(texture);
    render_texture_cube_array_construct(
        texture.descriptor_state,
        texture.cubes,
        descriptor,
        render_texture_descriptor_has_source_data(descriptor.texture_state));
    render_texture_cube_array_validate(texture.cubes);

    texture.resource_index = 2;
    const auto& first_face = texture.cubes.begin->faces[0].storage;
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
    write_value(
        texture.descriptor_state,
        112,
        static_cast<std::uint64_t>(texture.cubes.end - texture.cubes.begin));
    std::memcpy(
        &texture.descriptor_type,
        texture.descriptor_state,
        sizeof(texture.descriptor_state));
}

// Reconstructed from eboot.elf at 0x69AA60.
RenderTextureArrayCube* render_create_texture_array_cube(
    RenderTextureArrayCubeDescriptor& descriptor,
    RenderTextureArrayCube* reusable_texture) {
    render_texture_resolve_descriptor_fields(
        &descriptor.texture_state.usage_type,
        7,
        descriptor.texture_state.creation_state.values,
        -1);
    auto& factory = *render_system_factory(*render_system_instance());
    auto* texture = render_factory_create_texture_array_cube(
        factory, descriptor);
    render_texture_initialize_backend(*texture, reusable_texture);
    return texture;
}

// Reconstructed from eboot.elf at 0x69ACD0.
void render_texture_array_cube_destruct(RenderTextureArrayCube& texture) {
    render_texture_array_cube_set_base_dispatch(texture);
    render_texture_cube_array_destruct(texture.cubes);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x69AD10.
void render_texture_array_cube_delete(RenderTextureArrayCube& texture) {
    render_texture_array_cube_destruct(texture);
    render_delete_texture_array_cube_storage(texture);
}

}  // namespace rb4
