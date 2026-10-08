#include "render/core/textures/render_texture_cube.h"

#include <cstddef>
#include <cstring>

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

bool descriptor_has_source_data(
    const RenderTextureCubeDescriptor& descriptor) {
    return descriptor.texture_state.backend_initialized ||
           (descriptor.texture_state.flags & 5U) != 0;
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
        descriptor_has_source_data(descriptor));

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
