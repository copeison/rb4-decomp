#include "render/core/textures/render_texture_1d.h"

#include <cstddef>
#include <cstring>

#include "render/core/textures/render_texture_1d_adapters.h"

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
    const RenderTexture1DDescriptor& descriptor) {
    return descriptor.texture_state[124] != 0 ||
           (descriptor.texture_state[88] & 5U) != 0;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6F5870.
void render_texture_1d_construct(
    RenderTexture1D& texture,
    const RenderTexture1DDescriptor& descriptor) {
    render_texture_construct(texture);
    render_texture_1d_set_base_dispatch(texture);

    std::memcpy(
        texture.descriptor_state,
        descriptor.texture_state,
        sizeof(texture.descriptor_state));
    render_texture_mip_chain_construct(
        texture.mip_chain,
        descriptor.mip_source_state,
        descriptor_has_source_data(descriptor));

    write_value(
        texture.descriptor_state,
        92,
        read_value<std::uint32_t>(descriptor.mip_source_state, 20));
    write_value(
        texture.descriptor_state,
        96,
        read_value<std::uint64_t>(descriptor.mip_source_state, 8));
    write_value(
        texture.descriptor_state,
        104,
        read_value<std::uint32_t>(descriptor.mip_source_state, 16));
    std::memcpy(
        &texture.descriptor_type,
        texture.descriptor_state,
        sizeof(texture.descriptor_state));
}

// Reconstructed from eboot.elf at 0x6F5970.
void render_texture_1d_destruct(RenderTexture1D& texture) {
    render_texture_1d_set_base_dispatch(texture);
    render_texture_mip_chain_destruct(texture.mip_chain);
    render_texture_destruct(texture);
}

// Reconstructed from eboot.elf at 0x6F59B0.
void render_texture_1d_delete(RenderTexture1D& texture) {
    render_texture_1d_destruct(texture);
    render_delete_texture_1d_storage(texture);
}

}  // namespace rb4
